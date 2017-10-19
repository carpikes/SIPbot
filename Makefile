CC=gcc
CFLAGS=-g -Wall -pedantic
LDFLAGS=-leXosip2 -losip2 -lrt -lm -lpthread -losipparser2 -lortp
SOURCES=src/main.c src/sdp.c src/sip.c src/log.c src/law.c src/filter.c \
		src/call.c src/spawn.c src/cmds.c src/config.c
OBJECTS=$(SOURCES:.c=.o)
DOXYGEN=doxygen
EXECUTABLE=sipbot

all:
	$(CC) $(SOURCES) $(CFLAGS) -o $(EXECUTABLE) $(LDFLAGS)

audio-tests:
	$(CC) src/log.c src/law.c tests/test_wave.c src/filter.c $(CFLAGS) -o test_wave $(LDFLAGS)
	$(CC) -Wall -g tests/test_filter.c -o test_filter -lpulse -lpulse-simple -lm
	$(CC) -Wall -g tests/test_downsample.c src/filter.c -o test_downsample -lpulse -lpulse-simple -lm

doc:
	$(DOXYGEN) DoxyFile

clean:
	rm -f $(EXECUTABLE) test_wave test_filter test_downsample
	rm -rf docs/
