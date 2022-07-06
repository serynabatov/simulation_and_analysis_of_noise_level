package data.science;

import data.science.utilities.StructureType;
import lombok.val;
import org.apache.spark.api.java.function.VoidFunction2;
import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.SparkSession;
import org.apache.spark.sql.streaming.StreamingQueryException;
import org.apache.spark.sql.types.*;


import java.io.File;
import java.time.Instant;
import java.util.concurrent.TimeoutException;

import static org.apache.spark.sql.functions.*;

public class SensorStreamingContext {

    public static void main(String[] args) throws TimeoutException, StreamingQueryException {
        final String master = args.length > 0 ? args[0] : "local[4]";

        String path = "./spark/log/";
        File directory = new File(path);

        directory.mkdirs();

        String kafkaAddress = "192.168.43.55:9092";

        final SparkSession spark = SparkSession
                .builder()
                .master(master)
                .appName("StructuredStreamingWordCount")
                .getOrCreate();

        Dataset<Row> rawDf = spark
                .readStream()
                .format("kafka")
                .option("kafka.bootstrap.servers", kafkaAddress)
                .option("subscribe", "data-prepared")
                .load();

        StructType pointSchema = StructureType.pointSchemaCreate();

        Dataset<Row> pointSdf = rawDf.select(from_json(rawDf.col("value").cast("string"), DataType.fromJson(pointSchema.json())).as("data")).select(col("data.*"));

        StructType pointOfInterestSchema = StructureType.pointOfInterestSchemaCreate();

        Dataset<Row> noiseEvent = pointSdf.withColumn("pointOfInterest", explode(col("pointOfInterest"))).as("data")
                .select("data.*")
                .select(from_json(col("data.pointOfInterest").cast("string"), DataType.fromJson(pointOfInterestSchema.json())).as("point"),
                        col("latitude"),
                        col("longitude"),
                        col("db"),
                        col("exceeded"),
                        col("noiseLevel"),
                        col("timestamp"),
                        col("timeOfTheDay"))
                .select(col("point.*"),
                        col("latitude"),
                        col("longitude"),
                        col("db"),
                        col("exceeded"),
                        col("noiseLevel"),
                        col("timestamp"),
                        col("timeOfTheDay"))
                .dropDuplicates("latitude", "longitude", "db", "timestamp");

        /* Create view Effective Noise  */
        Dataset<Row> effectiveNoiseView = noiseEvent.select(to_timestamp(col("timestamp").divide(1000)).as("timestamp"), // TODO: in production change it as division by 60000
                                                                col("latitude"),
                                                                col("longitude"),
                                                                col("db"),
                                                                col("exceeded"),
                                                                col("noiseLevel"),
                                                                col("timeOfTheDay"),
                                                                col("id"),
                                                                col("lat").as("latitude_poi"),
                                                                col("lon").as("longitude_poi"),
                                                                col("name"))
                                                .withWatermark("timestamp", "1 seconds") // TODO: in production we should change it to 1 minute!!!
                                                .groupBy("timestamp", "noiseLevel", "timeOfTheDay",
                                                        "id", "latitude_poi", "longitude_poi"   , "name", "exceeded").
                                                max("db");

        // Q1
        // Hourly
        val hourlyQuery = effectiveNoiseView.
                writeStream()
                .outputMode("update")
                /*.format("kafka")
                .option("kafka.bootstrap.servers", kafkaAddress)
                .option("topic", "q1-hourly")*/
                .queryName("hourly")
                .foreachBatch((VoidFunction2<Dataset<Row>, Long>) (rowDataset, aLong) -> rowDataset
                        .withWatermark("timestamp", "1 seconds")   // TODO: in production change into 1 minute!/
                        .groupBy(col("id"),
                                col("name"),
                                col("noiseLevel"),
                                col("timeOfTheDay"),
                                col("latitude_poi"),
                                col("longitude_poi"),
                                hour(col("timestamp")),
                                date_format(col("timestamp"), "yyyy-MM-dd"))
                        .avg("max(db)")
                        .write()
                        .option("header", true)
                        .format("csv")
                        .option("path", path + "/" + "Q1-Hourly")
                        .mode("append")
                        .save())
                .start();

        // Daily
        val dailQuery = effectiveNoiseView
                .writeStream()
                /*.format("kafka")
                .option("kafka.bootstrap.servers", kafkaAddress)
                .option("topic", "q1-daily")*/
                .outputMode("append")
                .queryName("daily")
                .foreachBatch((VoidFunction2<Dataset<Row>, Long>) (rowDataset, aLong) -> rowDataset
                        .withWatermark("timestamp", "1 seconds")    // TODO: in production change into 1 minute!
                        .groupBy(col("id"),
                                col("name"),
                                col("noiseLevel"),
                                col("timeOfTheDay"),
                                col("latitude_poi"),
                                col("longitude_poi"),
                                date_format(col("timestamp"), "yyyy-MM-dd"))
                        .avg("max(db)")
                        .write()
                        .option("header", true)
                        .format("csv")
                        .option("path", path + "/" + "Q1-Daily")
                        .mode("append")
                        .save())
                .start();

        // Weekly
        val weeklyQuery = effectiveNoiseView
                .writeStream()
                /*.format("kafka")
                .option("kafka.bootstrap.servers", kafkaAddress)
                .option("topic", "q1-weekly")*/
                .outputMode("append")
                .queryName("weeklyQuery")
                .foreachBatch((VoidFunction2<Dataset<Row>, Long>) (rowDataset, aLong) -> rowDataset
                        .withWatermark("timestamp", "1 seconds") //TODO: in production change into 1 minute!
                        .groupBy(col("id"),
                                col("name"),
                                col("noiseLevel"),
                                col("timeOfTheDay"),
                                col("latitude_poi"),
                                col("longitude_poi"),
                                year(col("timestamp")),
                                weekofyear(col("timestamp")))
                        .avg("max(db)")
                        .write()
                        .option("header", true)
                        .format("csv")
                        .option("path", path + "/" + "Q1-Weekly")
                        .mode("append")
                        .save())
                .start();

        // Q2
        val topTenQuery = effectiveNoiseView
                .writeStream()
                .queryName("TopTen")
                /*.format("kafka")
                .option("kafka.bootstrap.servers", kafkaAddress)
                .option("topic", "q2-TopTen")*/
                .outputMode("append")
                .foreachBatch((VoidFunction2<Dataset<Row>, Long>) (rowDataset, aLong) -> rowDataset
                        .withWatermark("timestamp", "1 seconds")     // TODO: in production put 10 minutes!
                        .filter(col("timestamp").gt(current_timestamp().minus(expr("INTERVAL 1 HOUR"))))
                        .groupBy(col("id"),
                                col("name"),
                                col("noiseLevel"),
                                col("timeOfTheDay"),
                                col("latitude_poi"),
                                col("longitude_poi"))
                        .avg("max(db)")
                        .write()
                        .option("header", true)
                        .format("csv")
                        .option("path", path + "/" + "Q2-TopTen")
                        .mode("append")
                        .save())
                .start();

        // Q3
        // Exceeded noise view
        Dataset<Row> exceededNoise = effectiveNoiseView.filter(col("exceeded").equalTo(true));


        val longestStreakQuery = exceededNoise
                .writeStream()
                /*.format("kafka")
                .option("kafka.bootstrap.servers", kafkaAddress)
                .option("topic", "exceededNoise")*/
                .outputMode("append")
                .queryName("LongestStreak")
                .foreachBatch((VoidFunction2<Dataset<Row>, Long>) (rowDataset, aLong) -> rowDataset
                        .withWatermark("timestamp", "1 seconds")           // TODO: change it in production for 1 MONUTES
                        .select(col("timestamp").cast(DataTypes.LongType).as("timestamp"),
                                col("max(db)"),
                                col("noiseLevel"),
                                col("timeOfTheDay"),
                                col("id"),
                                col("latitude_poi"),
                                col("longitude_poi"),
                                col("name"),
                                col("exceeded")
                        )
                        .groupBy("max(db)", "noiseLevel", "timeOfTheDay",
                                "id", "latitude_poi", "longitude_poi", "name", "exceeded").
                        max("timestamp")   // last exceeded noise
                        .select(col("id"),
                                col("name"),
                                col("max(db)").as("db"),
                                to_timestamp(current_timestamp().cast(DataTypes.LongType).divide(1000).minus(col("max(timestamp)").divide(1000))).as("eventDate") // TODO: in production divide by 6000
                        )
                        .orderBy(col("max(timestamp)"))
                        .limit(1)
                        .write()
                        .option("header", true)
                        .format("csv")
                        .option("path", path + "/" + "Q3-Exceeded")
                        .mode("append")
                        .save())
                .start();

        spark.streams().awaitAnyTermination();

    }

}
