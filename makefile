
CXX = gcc
CXXFLAGS = -O2 -Wall -g

TARGET = ddbclient ddbserver
CLIENTSRC = $(wildcard ./client/*.c ./common/*.c)
SERVERSRC = $(wildcard ./server/*.c ./common/*.c)
TESTSRC   = $(wildcard ./test/*.c)
CLIENTOBJ = $(CLIENTSRC:.c=.o)
SERVEROBJ = $(SERVERSRC:.c=.o)
TESTOBJ   = $(TESTSRC:.c=.o)

all: $(TARGET)
ddbclient: $(CLIENTOBJ)
	$(CXX) -o $@ $^
ddbserver: $(SERVEROBJ)
	$(CXX) -o $@ $^ -ltokyocabinet

test:   $(TESTOBJ) ./server/tcDB.o ./common/dbProtocol.o
	$(CXX) -o ./test/testtcDB ./test/testtcDB.o ./server/tcDB.o -ltokyocabinet
	./test/testtcDB
	$(CXX) -o ./test/testProtocol ./test/testProtocol.o ./common/dbProtocol.o
	./test/testProtocol

.c.o:
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	$(RM) $(CLIENTOBJ) $(SERVEROBJ) $(TARGET)
	$(RM) *.*db

cleantest:
	$(RM) $(TESTOBJ) ./server/tcDB.o ./common/dbProtocol.o
	$(RM) ./test/testtcDB ./test/testProtocol
	$(RM) ./test/*.*db
