package data.science;

import org.apache.spark.sql.Dataset;
import org.apache.spark.sql.Row;
import org.apache.spark.sql.SparkSession;

import java.util.concurrent.TimeoutException;

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

        Dataset<Row> df = spark
                .readStream()
                .format("kafka")
                .option("kafka.bootstrap.servers", "localhost:9092")
                .option("subscribe", "data-prepared")
                .load();

        spark.close();
    }

}
