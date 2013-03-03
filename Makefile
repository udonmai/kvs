# Makefile
CC = gcc
CFLAGS = -g -Wall
CLIBS = -lpthread
OBJS_SERV = hash.o str.o kvs-server.o wrapunix.o readline.o daemon.o
OBJS_CLI = kvs-client.o wrapunix.o
SERV = kvs-server
CLI = kvs-client
BIN_DIR = bin

.SUFFIXES: .c .o

$(BIN_DIR) : $(SERV) $(CLI)

$(SERV): $(OBJS_SERV)
	$(CC) -o $(SERV) $(CLIBS) $(CFLAGS) $(OBJS_SERV)

$(CLI): $(OBJS_CLI)
	$(CC) -o $(CLI) $(CLIBS) $(CFLAGS) $(OBJS_CLI)

.c.o:
	$(CC) -c $(CFLAGS) $<

.PHONY: clean
clean:
	$(RM) $(SERV) $(CLI) $(OBJS_CLI) $(OBJS_SERV)

kvs-client.o: wrapunix.h
