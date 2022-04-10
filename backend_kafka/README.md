# How To Run it?

Go to the folder where kafka is stored and run the following command - it runs the zookeeper

```
bin/zookeeper-server-start.sh config/zookeeper.properties
```

Then in another terminal runs the broker

```
bin/kafka-server-start.sh config/server.properties
```

In another terminal create the topic, in our case it sensor-event

```
bin/kafka-topics.sh --create --partitions 1 --replication-factor 1 --topic sensor-event --bootstrap-server localhost:9092
```

After it you could run both producer/consumer (for now it works only in the test mode, unfortunately).
