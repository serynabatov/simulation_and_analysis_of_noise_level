package components;

import org.apache.kafka.clients.consumer.ConsumerConfig;
import org.apache.kafka.clients.producer.KafkaProducer;
import org.apache.kafka.clients.producer.ProducerRecord;
import org.apache.kafka.clients.producer.RecordMetadata;
import org.apache.kafka.common.serialization.StringDeserializer;

import java.util.Properties;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;

public class EnrichedProducer {

    private final KafkaProducer<String, String> producer;
    private final String publishTopic = "data-prepared";

    public EnrichedProducer(String serverAddr) {
        final Properties propertiesPublish = new Properties();
        propertiesPublish.put(ConsumerConfig.BOOTSTRAP_SERVERS_CONFIG, serverAddr);
        propertiesPublish.put(ConsumerConfig.KEY_DESERIALIZER_CLASS_CONFIG, StringDeserializer.class.getName());
        propertiesPublish.put(ConsumerConfig.VALUE_DESERIALIZER_CLASS_CONFIG, StringDeserializer.class.getName());
        producer = new KafkaProducer<String, String>(propertiesPublish);
    }


    public void publishMessage(String key, String value) {
        final ProducerRecord<String, String> record = new ProducerRecord<>(publishTopic, key, value);
        final Future<RecordMetadata> future = producer.send(record);

        try {
            RecordMetadata ack = future.get();
            System.out.println("Ack has been received for topic " + ack.topic() + " for partition " + ack.partition() + " for offset " + ack.offset());
        } catch (InterruptedException | ExecutionException e) {
            e.printStackTrace();
        }
    }
}
