CC=g++
CFLAGS += $(shell mpicxx -showme:compile)
CDFLAGS += $(shell pkg-config --cflags rdkafka++ libcjson)
LDFLAGS += $(shell mpicxx -showme:link)
LDCFLAGS += $(shell pkg-config --libs rdkafka++ libcjson)
SOURCES := utilities/stringutility.cpp utilities/basic_utils.cpp utilities/cjson_utils.cpp
HEADERS := -Iutilities



mpisim:
	$(CC) $(CFLAGS) $(CDFLAGS) $(HEADERS) -o znewsim $(SOURCES) newsim.cpp $(LDFLAGS) $(LDCFLAGS) 