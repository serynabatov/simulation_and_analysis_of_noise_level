package data.science;

import data.science.udt.PointOfInterestUDT;
import lombok.val;
import org.apache.spark.api.java.function.VoidFunction2;
import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.SparkSession;
import org.apache.spark.sql.streaming.StreamingQueryException;
import org.apache.spark.sql.types.*;


import java.util.concurrent.TimeoutException;

import static org.apache.spark.sql.functions.*;

public class SensorStreamingContext {

    // Link:
    //https://spark.apache.org/docs/latest/structured-streaming-kafka-integration.html
    
    public static void main(String[] args) throws TimeoutException, StreamingQueryException {
        final String master = args.length > 0 ? args[0] : "local[4]";

        final SparkSession spark = SparkSession
                .builder()
                .master(master)
                .appName("StructuredStreamingWordCount")
                .getOrCreate();

        Dataset<Row> raw_df = spark
                .readStream()
                .format("kafka")
                .option("kafka.bootstrap.servers", "localhost:9092")
                .option("subscribe", "data-prepared")
                .load();

        StructType point_schema = new StructType()
                .add("latitude", DataTypes.FloatType, true)
                .add("longitude", DataTypes.FloatType, true)
                .add("db", DataTypes.FloatType, true)
                .add("exceeded", DataTypes.BooleanType, true)
                .add("timestamp", DataTypes.LongType, true)
                .add("pointOfInterest", DataTypes.createArrayType(new PointOfInterestUDT()), true)
                .add("noiseLevel", DataTypes.StringType, true)
                .add("timeOfTheDay", DataTypes.StringType,  false);

        Dataset<Row> point_sdf = raw_df.select(from_json(raw_df.col("value").cast("string"), DataType.fromJson(point_schema.json())).as("data")).select(col("data.*"));

        //val basicQuery = point_sdf.writeStream().format("console").queryName("basic").start();

        StructType pointOfInterestSchema = new StructType()
                .add("id", DataTypes.IntegerType, true)
                .add("lat", DataTypes.FloatType, true)
                .add("lon", DataTypes.FloatType, true)
                .add("name", DataTypes.StringType, true);

        Dataset<Row> noiseEvent = point_sdf.withColumn("pointOfInterest", explode(col("pointOfInterest"))).as("data")
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
                        col("timeOfTheDay"));

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

        //val effectiveNoiseQuery = effectiveNoiseView.writeStream().format("console").queryName("effective").start();

        // Q1

        // Hourly

        val hourlyQuery = effectiveNoiseView.
                writeStream()
                .format("console")
                .queryName("hourly")
                .foreachBatch((VoidFunction2<Dataset<Row>, Long>) (rowDataset, aLong) -> rowDataset
                        .withWatermark("timestamp", "1 seconds")   // TODO: in production change into 1 minute!
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
                        .format("console")
                        .save())
                .start();

        // Daily

        val dailQuery = effectiveNoiseView
                .writeStream()
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
                        .format("console")
                        .save())
                .start();

        // Weekly

        val weeklyQuery = effectiveNoiseView
                .writeStream()
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
                        .format("console")
                        .save())
                .start();

        // Q2

        val topTenQuery = effectiveNoiseView
                .writeStream()
                .queryName("TopTen")
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
                        .format("console")
                        .save())
                .start();

        // Q3

        // Exceeded noise view
        Dataset<Row> exceededNoise = effectiveNoiseView.filter(col("exceeded").equalTo(true));


        val longestStreakQuery = exceededNoise
                .writeStream()
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
                        .format("console")
                        .save())
                .start();

        spark.streams().awaitAnyTermination();

    }

}
