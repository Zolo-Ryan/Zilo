kilo: kilo.c
	$(CC) $< -o $@ -Wall -Wextra -pedantic -std=c99

clean:
	rm kilo