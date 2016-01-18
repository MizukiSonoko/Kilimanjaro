CC=clang++
TARGET=sharo
OPTION=-std=c++0x -Wall -DDEBUG

all: lexer.o front.o parser.o
			$(CC) $(OPTION) lexer.o parser.o front.o -o $(TARGET)

lexer.o: lexer.cpp
			$(CC) $(OPTION) -c lexer.cpp -o lexer.o

front.o: front.cpp
			$(CC) $(OPTION) -c front.cpp -o front.o

parser.o: parser.cpp
			$(CC) $(OPTION) -c parser.cpp -o parser.o

clean:
			rm lexer.o front.o parser.o sharo

test: sharo
	./sharo sample.cocoa rule.moca