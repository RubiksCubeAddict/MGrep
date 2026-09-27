CC = gcc
CFLAGS = -Wall -pedantic -std=c99
TARGET = mgrep
SRC = src/main.c src/search.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) $(SRC) -o $(TARGET)

clean:
	rm -f $(TARGET)