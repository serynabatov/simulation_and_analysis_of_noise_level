CC=gcc
CFLAGS += $(shell mpicc -showme:compile)
LDFLAGS += $(shell mpicc -showme:link)

LDFLAGS += -L/home/sergey/projects/test/MQTT_Paho/paho.mqtt.c/build/output/ -lpaho-mqtt3c

mpisim:
	$(CC) $(CFLAGS) -o deneme deneme.c mqtt-client.h mqtt-client.c $(LDFLAGS)