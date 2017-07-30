all : TestSerialator TestSerialator2

CC=g++

TestSerialator : TestSerialator.cpp ../Serialator.cpp ../Serialator.h Makefile
	$(CC) TestSerialator.cpp ../Serialator.cpp -o TestSerialator -std=c++0x -I..

TestSerialator2 : TestSerialator2.cpp ../Serialator.cpp ../Serialator.h Makefile
	$(CC) TestSerialator2.cpp ../Serialator.cpp -o TestSerialator2 -std=c++0x -I..

clean: 
	rm TestSerialator
