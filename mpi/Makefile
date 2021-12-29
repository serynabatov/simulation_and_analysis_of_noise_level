
CC=gcc
CFLAGS += $(shell mpicc -showme:compile)
LDFLAGS += $(shell mpicc -showme:link)

LDFLAGS += -L/home/egesaygili/Desktop/paho.mqtt.c/build/output/ -lpaho-mqtt3c

mpisim:
	$(CC) $(CFLAGS) -o sim sim_mult_regs.c mqtt-client.h mqtt-client.c $(LDFLAGS)