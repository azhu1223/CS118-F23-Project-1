CC=gcc
SRC=server.cpp
OBJ=server
CFLAGS=-fsanitize=address

all: $(OBJ)

proxy: $(SRC)
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJ)

.PHONY: all clean
