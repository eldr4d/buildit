CFLAGS = -std=c++11 -O6
LFLAGS = -L/usr/local/lib/ -lboost_iostreams -lboost_serialization


all: append.o read.o logmanager.o blowfish.o
	g++ $(CFLAGS) -o logappend append.o logmanager.o blowfish.o $(LFLAGS)
	g++ $(CFLAGS) -o logread read.o logmanager.o blowfish.o $(LFLAGS)

append.o: append.cpp
	g++ $(CFLAGS) -c append.cpp

read.o: read.cpp
	g++ $(CFLAGS) -c read.cpp

logmanager.o: logmanager.cpp logmanager.hpp printHTML.hpp
	g++ $(CFLAGS) -c logmanager.cpp logmanager.hpp printHTML.hpp

blowfish.o: blowfish.cpp blowfish.hpp
	g++ $(CFLAGS) -c blowfish.cpp blowfish.hpp


clean:
	rm -r *.o *.gch logappend logread