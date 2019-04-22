CC=gcc
CFLAGS= -std=gnu99  -Wextra -pedantic
LFLAGS= -lpthread
proj2: proj2.c
	$(CC) $(CFLAGS) $(LFLAGS) -g proj2.c -o proj2