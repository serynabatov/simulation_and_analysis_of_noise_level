# Before running Kafka

You need to download

- [Kafka broker, zookeper software](https://kafka.apache.org/downloads)
- [H2 database](http://www.h2database.com/html/download.html)

All software was tested using the **following versions**:

- Kafka - 3.0.0
- H2 - 2022-04-09
- Ubuntu Linux - 20.04.3 LTS (focal)
- Java - openjdk-11/18

After you have downloaded Kafka, and unarchive it, you should run the following commands:

## Run Zookeeper
```
sudo ./bin/zookeeper-server-start.sh ./config/zookeeper.properties
```

## Run Kafka Server

Uncomment in ./config/server.properties two lines with listeners (listeners & advertised.listeners) and put in advertised.listeners instead of PLAIN_TEXT the IP address that you need

![photo_2022-06-28_21-06-27](https://user-images.githubusercontent.com/23494724/176289674-6e2463b6-ce22-4656-a147-5cfbad7efc18.jpg)

```
sudo ./bin/kafka-server-start.sh ./config/server.properties
```

## Create topics
```
sudo ./bin/kafka-topics.sh --create --topic sensor-readings --bootstrap-server localhost:9092 --partitions 3 --replication-factor 2
```

```
sudo ./bin/kafka-topics.sh --create --topic data-prepared --bootstrap-server localhost:9092 --partitions 3 --replication-factor 2
```

## Run H2
After everything was run, double check that the H2 Database is fine, under /bin, run the following command:

```
./h2.sh
```

**Doesn't close the database it will be needed later!**

# Run application

Run the content of **./database.sql** inside the H2 web browser window.

Then you can run the BasicConsumer.java and the system will start to work

# TODO

- Automatize the population of database
- Automatize the environment
- Do system tests
