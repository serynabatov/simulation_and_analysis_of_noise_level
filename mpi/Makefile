CC=g++
CFLAGS += $(shell mpicxx -showme:compile)
CDFLAGS += $(shell pkg-config --cflags rdkafka++ libcjson)
LDFLAGS += $(shell mpicxx -showme:link)
LDCFLAGS += $(shell pkg-config --libs rdkafka++ libcjson)


mpisim:
	$(CC) $(CFLAGS) $(CDFLAGS) -o znewsim newsim.cpp $(LDFLAGS) $(LDCFLAGS)