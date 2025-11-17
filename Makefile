CC=gcc
CFLAGS=-Wall -Wextra -g

SRC=src/main.c src/proc_parser.c
OBJ=$(SRC:.c=.o)
OUT=mini_top

all: $(OUT)

$(OUT): $(OBJ)
	$(CC) $(CFLAGS) -o $(OUT) $(OBJ)

clean:
	rm -f $(OBJ) $(OUT)
