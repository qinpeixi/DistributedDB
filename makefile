
CXX = gcc
CXXFLAGS = -O2 -Wall -g

TARGET = ddbclient ddbserver
TESTTARGETS = testtcDB testProtocol
CLIENTSRC = $(wildcard ./client/*.c ./common/*.c)
SERVERSRC = $(wildcard ./server/*.c ./common/*.c)
CLIENTOBJ = $(CLIENTSRC:.c=.o)
SERVEROBJ = $(SERVERSRC:.c=.o)

all: $(TARGET)
ddbclient: $(CLIENTOBJ)
	$(CXX) -o $@ $^
ddbserver: $(SERVEROBJ)
	$(CXX) -o $@ $^ -ltokyocabinet

.c.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	$(RM) $(CLIENTOBJ) $(SERVEROBJ) $(TARGET)
	$(RM) *.*db
