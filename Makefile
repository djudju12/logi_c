main: main.c
	gcc -Wall -Wextra -pedantic -std=c99 -g main.c -o logic

run: main
	./logic

debug: main
	/home/jonathan/Software/gf/gf2 logic &