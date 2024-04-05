main: main
	gcc -Wall -Wextra -pedantic -std=c99 -g main.c -o logic

run: main
	./logic

debug:
	/home/jonathan/Software/gf/gf2 logic &