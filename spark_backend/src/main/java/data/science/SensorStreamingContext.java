package data.science;

import data.science.udt.PointOfInterestUDT;
import lombok.val;
import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.SparkSession;
import org.apache.spark.sql.streaming.StreamingQueryException;
import org.apache.spark.sql.types.*;


import java.util.concurrent.TimeoutException;

import static org.apache.spark.sql.functions.col;
import static org.apache.spark.sql.functions.from_json;

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

        val basicQuery = point_sdf.writeStream().format("console").queryName("fuck").start();

        spark.streams().awaitAnyTermination();

    }

}
