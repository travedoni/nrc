CC = gcc
CFLAGS = -Wall -g

SOURCES = src/main.c src/shell.c src/builtins.c src/variables.c src/parser.c
OBJECTS = $(SOURCES:.c=.o)
EXECUTABLE = nrc

all: $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(CFLAGS) -o $@ $^

src/%.o: src/%.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJECTS) $(EXECUTABLE)

.PHONY: all clean
