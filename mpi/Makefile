
CC=gcc
CFLAGS += $(shell mpicc -showme:compile)
LDFLAGS += $(shell mpicc -showme:link)

LDFLAGS += -L/home/sergey/projects/test/MQTT_Paho/paho.mqtt.c/build/output -lpaho-mqtt3a

mpisim:
	$(CC) $(CFLAGS) -o sim sim_mult_regs.c mqtt-client.h mqtt-client.c $(LDFLAGS) -luuid