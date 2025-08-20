CFLAGS := -std=c++20 -g -Wall -Wextra -Wpedantic -fsanitize=address,undefined

SRCS := $(shell find src -type f -name "*.cpp")

BIN := bin/yadda

run: $(BIN)
	./$^ src/app.hpp

$(BIN): $(SRCS)
	g++ -DNDEBUG $(CFLAGS) -o $@ $^

clean:
	rm -rf bin/*

