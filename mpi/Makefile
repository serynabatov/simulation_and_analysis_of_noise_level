CC=g++
CFLAGS += $(shell mpicxx -showme:compile)
CDFLAGS += $(shell pkg-config --cflags rdkafka++)
LDFLAGS += $(shell mpicxx -showme:link)
LDCFLAGS += $(shell pkg-config --libs rdkafka++)

mpisim:
	$(CC) $(CFLAGS) $(CDFLAGS) -o newsim newsim.cpp $(LDFLAGS) $(LDCFLAGS)