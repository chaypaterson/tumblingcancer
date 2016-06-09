CC = g++

SRC = tumblera.cpp
BIN = tumblera

FLAGS = -std=c++11 -Wall -O3 -g
PREFIX = ~/.local

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

