CC = gcc
CFLAGS = -Wall -Wextra -Werror -std=c99

TARGET = search_index
SRC = tema2.c

build:
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)
clean:
	rm -f $(TARGET)