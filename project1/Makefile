TARGET = 3600sh

$(TARGET): $(TARGET).c
	gcc -std=c99 -O0 -g -lm -Wall -pedantic -Werror -Wextra -o $@ $<

all: $(TARGET)

test: all
	./test

ourTest: all
	./ourTest

clean:
	rm $(TARGET)

