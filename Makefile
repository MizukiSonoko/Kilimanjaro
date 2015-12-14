CC=clang++
TARGET=sharo
OPTION=-std=c++0x -Wall

all: sharo.o
			$(CC) sharo.o -o $(TARGET)

sharo.o: lexer.cpp
			$(CC) $(OPTION) -c lexer.cpp

clean:
			rm sharo.o sharo

