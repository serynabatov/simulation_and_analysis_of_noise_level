package data.science;

import data.science.entıtıes.PointOfInterest;
import data.science.udt.PointOfInterestUDT;
import lombok.val;
import org.apache.spark.api.java.function.ForeachFunction;
import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.SparkSession;
import org.apache.spark.sql.streaming.StreamingQueryException;
import org.apache.spark.sql.types.*;
import org.apache.spark.streaming.Duration;
import org.apache.spark.streaming.Durations;
import org.apache.spark.streaming.StreamingContext;

import java.util.concurrent.TimeoutException;

import static org.apache.spark.sql.functions.from_json;
import static org.apache.spark.sql.types.DataTypes.createArrayType;

public class SensorStreamingContext {

    // Link:
    //https://spark.apache.org/docs/latest/structured-streaming-kafka-integration.html
    
    public static void main(String[] args) throws TimeoutException {
        final String master = args.length > 0 ? args[0] : "local[4]";

        final SparkSession spark = SparkSession
                .builder()
                .master(master)
                .appName("StructuredStreamingWordCount")
                .getOrCreate();

        val sc = spark.sparkContext();
        val ssc = new StreamingContext(sc, Durations.seconds(1));

        Dataset<Row> raw_df = spark
                .readStream()
                .format("kafka")
                .option("kafka.bootstrap.servers", "localhost:9092")
                .option("subscribe", "data-prepared")
                .load();

        raw_df.selectExpr("CAST(key AS STRING)", "CAST(value AS STRING)");

        raw_df.writeStream().format("memory").queryName("d").start();

        val point_schema = new StructType()
                .add("x", DataTypes.FloatType, true)
                .add("y", DataTypes.FloatType, true)
                /*.add("db", DataTypes.FloatType, true)
                .add("exceeded", DataTypes.BooleanType, true)
                .add("timestamp", DataTypes.StringType, true)
                .add("pointOfInterest", DataTypes.createArrayType(new PointOfInterestUDT()), true)
                .add("noiseLevel", DataTypes.StringType, true)
                .add("timeOfTheDay", DataTypes.StringType, true)*/;

        Dataset<Row> point_sdf = raw_df.select(from_json(raw_df.col("value").cast("string"), DataType.fromJson(point_schema.json())).as("data")).select("data.*");

        val basicQuery = point_sdf.writeStream().format("memory").queryName("fuck").start();

        while (true) {
            spark.sql("SELECT * FROM fuck ORDER BY timestamp").show(5);
            //spark.sql("SELECT value FROM d").show();
        }

        //spark.close();
    }

}
