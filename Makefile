CC=gcc
CFLAGS=-g -Wall -pedantic
LDFLAGS=-leXosip2 -losip2 -lrt -lm -lpthread -losipparser2 -lortp
SOURCES=src/main.c src/sdp.c src/sip.c src/log.c
OBJECTS=$(SOURCES:.c=.o)
DOXYGEN=doxygen
EXECUTABLE=sipbot

all: 
	$(CC) $(LDFLAGS) $(SOURCES) $(CFLAGS) -o $(EXECUTABLE)

doc:
	$(DOXYGEN) DoxyFile
