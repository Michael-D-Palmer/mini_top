CC = gcc
CFLAGS = -Wall -Wextra -g -pthread
SRC = src/main.c src/proc_parser.c src/cpu_sampler.c
OBJ = $(SRC:.c=.o)
TARGET = mini_top

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJ)
	$(CC) $(CFLAGS) -o $@ $^ -lncurses

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) $(TARGET)
