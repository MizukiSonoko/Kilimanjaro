CC=clang++
TARGET=sharo
OPTION=-std=c++0x -Wall

all: lexer.o front.o
			$(CC) front.o lexer.o -o $(TARGET)

lexer.o: lexer.cpp
			$(CC) $(OPTION) -c lexer.cpp -o lexer.o

front.o: front.cpp
			$(CC) $(OPTION) -c front.cpp -o front.o

clean:
			rm lexer.o front.o sharo

