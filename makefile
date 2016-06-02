CC = g++

SRC = tumblera.cpp
BIN = tumblera

FLAGS = -std=c++0x -Wall -O3
PREFIX = ~/.locaL

all: options tumblera

options:
	@echo tumblera build options:
	@echo "FLAGS = $(FLAGS)"
	@echo "CC    = $(CC)"

tumblera : $(SRC)
	$(CC) $(SRC) -o $(BIN) $(FLAGS) 

install: all
	@echo installing executable file to $(PREFIX)/bin
	@mkdir -p $(PREFIX)/bin
	@cp -f $(BIN) $(PREFIX)/bin
	@chmod 755 $(PREFIX)/bin/$(BIN)

