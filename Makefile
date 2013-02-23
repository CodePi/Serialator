TestSerializer : TestSerializer.cpp Serializer.cpp Serializer.h Makefile
	g++ TestSerializer.cpp Serializer.cpp -o TestSerializer -std=c++0x

clean: 
	rm TestSerializer
