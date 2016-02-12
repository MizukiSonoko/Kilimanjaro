CC=clang++
TARGET=sharo
OPTION=-std=c++ly -Wall -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS
INCLUDE=-I/usr/local/Cellar/llvm/3.6.2/include
LIBS=-L/usr/local/Cellar/llvm/3.6.2/lib
all: lexer.o parser.o
			$(CC) lexer.o parser.o -o $(TARGET)

lexer.o: lexer.cpp
			$(CC) $(OPTION) -c lexer.cpp -o lexer.o

parser.o: parser.cpp
			$(CC) $(OPTION) -c parser.cpp -o parser.o

clean:
			rm sharo.o sharo

test:
	./sharo in

llvm:
	clang++ $(OPTION) -o kirima `llvm-config --system-libs --libs core --cxxflags --ldflags` llvm.cpp

