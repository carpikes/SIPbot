CC=gcc
CFLAGS=-g -Wall -pedantic
LDFLAGS=-leXosip2 -losip2 -lrt -lm -lpthread -losipparser2 -lortp
SOURCES=main.c sdp.c sip.c log.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=sipbot

all: 
	$(CC) $(LDFLAGS) $(SOURCES) $(CFLAGS) -o $(EXECUTABLE)
