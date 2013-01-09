
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

TESTGROUP = slave master
SLAVEOBJ  = ./common/dbProtocol.o ./common/Socket.o ./server/slave.o
MASTEROBJ = ./common/dbProtocol.o ./common/Socket.o ./server/master.o

RUBBISHFILES = find . -regex '.*\.gch\|.*~\|.*\..*db' -type f

#all: $(TARGET)
all: $(TESTGROUP)
ddbclient: $(CLIENTOBJ)
	$(ECHO) $(CXX) -o $@ $^
ddbserver: $(SERVEROBJ)
	$(ECHO) $(CXX) -o $@ $^ -ltokyocabinet
	$(ECHO) mkdir server1 server2 server3
	$(ECHO) cp ddbserver ./server1/
	$(ECHO) cp ddbserver ./server2/
	$(ECHO) cp ddbserver ./server3/

test:   $(TESTOBJ) 
	$(ECHO) $(CXX) -o ./test/testtcDB ./test/testtcDB.o ./server/tcDB.o -ltokyocabinet
	$(ECHO) ./test/testtcDB
	$(ECHO) $(CXX) -o ./test/testProtocol ./test/testProtocol.o ./common/dbProtocol.o
	$(ECHO) ./test/testProtocol
	$(ECHO) $(CXX) -o ./test/testMemory ./test/testMDB.o ./server/MemoryDB.o -ltokyocabinet
	$(ECHO) ./test/testMemory
	$(ECHO) $(CXX) -o ./test/testCircularQueue ./test/testCircularQueue.o ./server/CircularQueue.o
	$(ECHO) ./test/testCircularQueue

testgroup: $(TESTGROUP)
slave:	$(SLAVEOBJ)
	$(ECHO) $(CXX) -o $@ $^ -lpthread
master: $(MASTEROBJ)
	$(ECHO) $(CXX) -o $@ $^ -ltokyocabinet
	$(ECHO) mkdir server1 server2 server3
	$(ECHO) cp slave ./server1/
	$(ECHO) cp slave ./server2/
	$(ECHO) cp slave ./server3/

.c.o:
	$(ECHO) $(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	@$(RM) $(CLIENTOBJ) $(SERVEROBJ) $(TARGET) $(TESTGROUP)
	@$(RM) -r ./server1 ./server2/ ./server3/
	@$(RUBBISHFILES) | xargs $(RM)
	@$(RM) *.o

cleantest:
	@$(RM) $(TESTOBJ) $(TESTTARGET)
	@$(RUBBISHFILES) | xargs $(RM)
