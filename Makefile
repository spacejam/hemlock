all: hemlock

hemlock: framework executor

framework:
	g++ -std=c++11 src/hemlock.cpp -o hemlock /usr/local/lib/libmesos.dylib

executor:
	g++ -std=c++11 src/hemlock_executor.cpp -o hemlock_executor /usr/local/lib/libmesos.dylib

clean:
	rm hemlock hemlock_executor
