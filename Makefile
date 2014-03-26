CC=gcc
CFLAGS=-g -Wall -pedantic
LDFLAGS=-leXosip2 -losip2 -lrt -lm -lpthread -losipparser2 -lortp
SOURCES=src/main.c src/sdp.c src/sip.c src/log.c src/law.c src/filter.c src/call.c
OBJECTS=$(SOURCES:.c=.o)
DOXYGEN=doxygen
EXECUTABLE=sipbot

all: 
	$(CC) $(LDFLAGS) $(SOURCES) $(CFLAGS) -o $(EXECUTABLE)

audio-tests:
	$(CC) $(LDFLAGS) src/log.c src/law.c tests/test_wave.c src/filter.c $(CFLAGS) -o test_wave	
	$(CC) -Wall -g -lpulse -lpulse-simple -lm tests/test_filter.c -o test_filter
	$(CC) -Wall -g -lpulse -lpulse-simple -lm tests/test_downsample.c src/filter.c -o test_downsample

doc:
	$(DOXYGEN) DoxyFile

clean:
	rm -f $(EXECUTABLE) test_wave test_filter test_downsample
	rm -rf docs/
