TARGET=main
CC=g++
WFLAGS=-O2 -Wall -Wextra -pedantic -Wshadow -Wconversion
CPPFLAGS=--std=c++17 -pthread $(WFLAGS)
PARSERDIR=src/parsers
UTILDIR=src/utilities
COMPDIR=src/components
OBJS=src/main.o \
	 $(UTILDIR)/print.o $(UTILDIR)/argparse.o $(UTILDIR)/timer.o $(UTILDIR)/checks.o \
	 $(COMPDIR)/grammar.o $(COMPDIR)/descriptor.o $(COMPDIR)/epn.o $(COMPDIR)/parser.o \
	 $(PARSERDIR)/sequential/sequential_parser.o \
	 $(PARSERDIR)/parallel_pool/parallel_pool.o \
	 $(PARSERDIR)/parallel_tree/parallel_tree.o

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CPPFLAGS) -o $(TARGET) $(OBJS)

main.o:
	$(CC) $(CPPFLAGS) -c main.cpp

sequential_parser.o: sequential_parser.hpp
	$(CC) $(CPPFLAGS) -c sequential_parser.cpp

parallel_pool.o: parallel_pool.hpp
	$(CC) $(CPPFLAGS) -c parallel_pool.cpp

parallel_tree.o: parallel_tree.hpp
	$(CC) $(CPPFLAGS) -c parallel_tree.cpp

print.o: print.hpp
	$(CC) $(CPPFLAGS) -c print.cpp

argparse.o: argparse.hpp
	$(CC) $(CPPFLAGS) -c argparse.cpp

timer.o: timer.hpp
	$(CC) $(CPPFLAGS) -c timer.cpp

checks.o: checks.hpp
	$(CC) $(CPPFLAGS) -c checks.cpp

grammar.o: grammar.hpp
	$(CC) $(CPPFLAGS) -c grammar.cpp

epn.o: epn.hpp
	$(CC) $(CPPFLAGS) -c epn.cpp

descriptor.o: descriptor.hpp
	$(CC) $(CPPFLAGS) -c descriptor.cpp

parser.o: parser.hpp
	$(CC) $(CPPFLAGS) -c parser.cpp

clean:
	rm -f $(TARGET) $(OBJS)