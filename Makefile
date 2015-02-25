CC = g++
override CFLAGS += -std=c++11
COMMON = /usr/local/lib/libmesos.dylib

all: hemlock

hemlock: framework executor storage

framework:
	$(CC) $(CFLAGS) $(COMMON) src/framework.cpp -o framework

executor:
	$(CC) $(CFLAGS) $(COMMON) src/executor.cpp -o executor

service:
	$(CC) $(CFLAGS) $(COMMON) 				\
		src/storage.cpp 								\
		-isrc/rocksdb/db.h 							\
		/usr/local/lib/librocksdb.dylib \
		-o service

clean:
	rm framework executor storage
