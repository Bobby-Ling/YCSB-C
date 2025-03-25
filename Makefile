

ROCKSDB_INCLUDE=/home/ubuntu/rocksdb-cloud/include              #Rocksdb的头文件
ROCKSDB_LIBRARY=/home/ubuntu/rocksdb-cloud/build/librocksdb.a   #Rocksdb的静态链接库
ROCKSDB_LIB=/home/ubuntu/rocksdb-cloud/build/
HDR_LIB=/usr/local/lib
AWS_INCLUDE=/usr/local/include/

CC=g++
CFLAGS=-std=c++20 -g -Wall -pthread -I./ -I$(ROCKSDB_INCLUDE) -I$(AWS_INCLUDE) -L$(ROCKSDB_LIB) -DUSE_AWS
#LDFLAGS= -lpthread -lrocksdb -lz -lbz2 -llz4 -ldl -lsnappy -lpmem -lnuma -lzstd
LDFLAGS= -lpthread -lz -lbz2 -llz4 -ldl -lsnappy -L$(HDR_LIB) -lzstd ${ROCKSDB_LIBRARY} -laws-cpp-sdk-s3 -laws-cpp-sdk-core -laws-cpp-sdk-transfer -laws-cpp-sdk-kinesis 
SUBDIRS= core db 
SUBSRCS=$(wildcard core/*.cc) $(wildcard db/*.cc)
OBJECTS=$(SUBSRCS:.cc=.o)
EXEC=ycsbc

all: $(SUBDIRS) $(EXEC)

$(SUBDIRS):
	#$(MAKE) -C $@
	$(MAKE) -C $@ ROCKSDB_INCLUDE=${ROCKSDB_INCLUDE} ROCKSDB_LIBRARY=${ROCKSDB_LIBRARY}

$(EXEC): $(wildcard *.cc) $(OBJECTS)
	$(CC) $(CFLAGS) $^ $(LDFLAGS) -o $@

clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir $@; \
	done
	$(RM) $(EXEC)

.PHONY: $(SUBDIRS) $(EXEC)

