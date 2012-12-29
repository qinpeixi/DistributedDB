
CXX = gcc
CXXFLAGS = -O2 -Wall -g
ECHO = @

TARGET = ddbclient ddbserver
CLIENTSRC = $(wildcard ./client/*.c ./common/*.c)
SERVERSRC = $(wildcard ./server/*.c ./common/*.c)
CLIENTOBJ = $(CLIENTSRC:.c=.o)
SERVEROBJ = $(SERVERSRC:.c=.o)

TESTTARGET = ./test/testtcDB ./test/testProtocol ./test/testMemory ./test/testCircularQueue
TESTSRC   = $(wildcard ./test/*.c)
TESTOBJ   = $(TESTSRC:.c=.o) ./server/tcDB.o ./common/dbProtocol.o ./server/MemoryDB.o \
		./server/CircularQueue.o

RUBBISHFILES = find . -regex '.*\.gch\|.*~\|.*\..*db' -type f

all: $(TARGET)
ddbclient: $(CLIENTOBJ)
	$(ECHO) $(CXX) -o $@ $^
ddbserver: $(SERVEROBJ)
	$(ECHO) $(CXX) -o $@ $^ -ltokyocabinet

test:   $(TESTOBJ) 
	$(ECHO) $(CXX) -o ./test/testtcDB ./test/testtcDB.o ./server/tcDB.o -ltokyocabinet
	$(ECHO) ./test/testtcDB
	$(ECHO) $(CXX) -o ./test/testProtocol ./test/testProtocol.o ./common/dbProtocol.o
	$(ECHO) ./test/testProtocol
	$(ECHO) $(CXX) -o ./test/testMemory ./test/testMDB.o ./server/MemoryDB.o -ltokyocabinet
	$(ECHO) ./test/testMemory
	$(ECHO) $(CXX) -o ./test/testCircularQueue ./test/testCircularQueue.o ./server/CircularQueue.o
	$(ECHO) ./test/testCircularQueue

.c.o:
	$(ECHO) $(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	@$(RM) $(CLIENTOBJ) $(SERVEROBJ) $(TARGET)
	@$(RUBBISHFILES) | xargs $(RM)
	@$(RM) *.o

cleantest:
	@$(RM) $(TESTOBJ) $(TESTTARGET)
	@$(RUBBISHFILES) | xargs $(RM)
