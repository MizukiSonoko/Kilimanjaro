CC=clang++
TARGET=sharo
OPTION=-g -O0 -std=c++1y -Wall -DDEBUG
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
