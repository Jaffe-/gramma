CC = g++
CFLAGS = -g -Wall -std=c++14
TARGET = grammar
SRCS = grammar.cpp
OBJS = $(SRCS: .c=.o)


all: $(TARGET)

$(TARGET) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm *.o *~ $(TARGET)

