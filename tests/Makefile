CC=g++
FLAGS=-std=c++0x -I..
TARGETS := TestSerialator TestSerialator2

all : $(TARGETS)

% : %.cpp ../Serialator.h ../Serialator.cpp
	$(CC) $< -o $@ $(FLAGS) ../Serialator.cpp

clean:
	rm -f $(TARGETS)
