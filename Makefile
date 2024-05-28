CC=gcc
CFLAGS=-Wall -Wextra -pedantic -std=c99 -g
LIBS=
PROGRAM_NAME=logic

BUILD_FOLDER=build
SRC_FOLDER=src

build: src/
	$(CC) $(CFLAGS) $(SRC_FOLDER)/main.c -o $(BUILD_FOLDER)/${PROGRAM_NAME} $(LIBS)

src/:
	mkdir -p $(BUILD_FOLDER)

run: build
	$(BUILD_FOLDER)/$(PROGRAM_NAME) -i

debug: build
	/home/jonathan/Software/gf/gf2 build/logic teste.lc &

run/file: build
	$(BUILD_FOLDER)/$(PROGRAM_NAME) teste.lc