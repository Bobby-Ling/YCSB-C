CC=g++
CFLAGS=-std=c++20 -c -g -Wall -fgnu-tm -lrocksdb
INCLUDES=-I../ -I${ROCKSDB_INCLUDE} -L${ROCKSDB_LIBRARY}
SOURCES=$(wildcard *.cc)
OBJECTS=$(SOURCES:.cc=.o)

all: $(SOURCES) $(OBJECTS)

.cc.o:
	$(CC) $(CFLAGS) $(INCLUDES) $< -o $@

clean:
	$(RM) $(OBJECTS)

