CC=clang++
TARGET=sharo
OPTION=--std=c++1y -Wall -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS
OPTLLVM=`llvm-config --system-libs --libs core --cxxflags --ldflags`
INCLUDE=-I/usr/local/Cellar/llvm/3.6.2/include
LIBS=-L/usr/local/Cellar/llvm/3.6.2/lib

all: lexer.o kirima.o front.o parser.o
			$(CC) $(OPTION) lexer.o parser.o kirima.o front.o -o $(TARGET)

lexer.o: lexer.cpp
			$(CC) $(OPTION) -c lexer.cpp -o lexer.o

front.o: front.cpp
			$(CC) $(OPTION) -c front.cpp -o front.o

parser.o: parser.cpp
			$(CC) $(OPTION) -c parser.cpp -o parser.o

test:
	./sharo in

kirima.o: kirima.o 
	$(CC) $(OPTION) $(OPTLLVM) -o kirima.o llvm.cpp

clean:
	rm *.o