CC=clang++
TARGET=sharo
OPTION=-std=c++0x -Wall
all: front.o peg.o
			$(CC) $(OPTION) peg.o front.o -o $(TARGET)


front.o: front.cpp
			$(CC) $(OPTION) -c front.cpp -o front.o

peg.o: peg.cpp
			$(CC) $(OPTION) -c peg.cpp -o peg.o

clean:
			rm front.o peg.o sharo

test: sharo
	./sharo sample.cocoa rule.moca