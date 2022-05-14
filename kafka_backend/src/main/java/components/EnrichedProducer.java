package components;

import org.apache.kafka.clients.producer.KafkaProducer;
import org.apache.kafka.clients.producer.ProducerConfig;
import org.apache.kafka.clients.producer.ProducerRecord;
import org.apache.kafka.clients.producer.RecordMetadata;
import org.apache.kafka.common.serialization.StringSerializer;

import java.util.Properties;
import java.util.concurrent.ExecutionException;
import java.util.concurrent.Future;

public class EnrichedProducer {

    private final KafkaProducer<String, String> producer;
    private final String publishTopic = "data-prepared";

    public EnrichedProducer(String serverAddr) {
        final Properties propertiesPublish = new Properties();
        propertiesPublish.put(ProducerConfig.BOOTSTRAP_SERVERS_CONFIG, serverAddr);
        propertiesPublish.put(ProducerConfig.KEY_SERIALIZER_CLASS_CONFIG, StringSerializer.class.getName());
        propertiesPublish.put(ProducerConfig.VALUE_SERIALIZER_CLASS_CONFIG, StringSerializer.class.getName());
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
