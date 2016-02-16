CC=clang++
TARGET=sharo
OPTION=--std=c++1y -Wall -D__STDC_CONSTANT_MACROS -D__STDC_LIMIT_MACROS
OPTLLVM=`llvm-config --system-libs --libs core --cxxflags --ldflags`
INCLUDE=-I/usr/local/Cellar/llvm/3.6.2/include
LIBS=-L/usr/local/Cellar/llvm/3.6.2/lib

all: lexer.o front.o parser.o
			$(CC) $(OPTION) $(OPTLLVM) lexer.o parser.o front.o -o $(TARGET)

lexer.o: lexer.cpp
			$(CC) $(OPTION) $(OPTLLVM) -c lexer.cpp -o lexer.o

front.o: front.cpp
			$(CC) $(OPTION) $(OPTLLVM) -c front.cpp -o front.o

test:
	./sharo in

llvm:
	$(CC) $(OPTION) $(OPTLLVM) -o kirima llvm.cpp
