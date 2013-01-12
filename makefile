
CXX = gcc
CXXFLAGS = -Wall -g
ECHO = @

TARGET = ddbclient ddbserver
CLIENTSRC = $(wildcard ./client/*.c ./common/*.c ./server/printslave.c)
SERVERSRC = $(wildcard ./server/*.c ./common/*.c ./client/inputcmd.c ./client/parseinput.c)
CLIENTOBJ = $(CLIENTSRC:.c=.o)
SERVEROBJ = $(SERVERSRC:.c=.o)

TESTTARGET = ./test/testtcDB ./test/testProtocol ./test/testMemory ./test/testCircularQueue
TESTSRC   = $(wildcard ./test/*.c)
TESTOBJ   = $(TESTSRC:.c=.o) ./server/tcDB.o ./common/dbProtocol.o ./server/MemoryDB.o \
		./server/CircularQueue.o

TESTGROUP = master
SLAVEOBJ  = ./common/dbProtocol.o ./common/Socket.o ./server/slave.o ./server/tcDB.o
MASTEROBJ = ./common/dbProtocol.o ./common/Socket.o ./server/master.o

RUBBISHFILES = find . -regex '.*\.gch\|.*~\|.*\..*db\|.*\.bac' -type f

all: $(TARGET)
#all: $(TESTGROUP)
ddbclient: $(CLIENTOBJ)
	$(ECHO) $(CXX) -o $@ $^ 
ddbserver: $(SERVEROBJ)
	$(ECHO) $(CXX) -o $@ $^ -ltokyocabinet
	#$(ECHO) mkdir server1 server2 server3
	$(ECHO) cp ddbserver ./server1/
	$(ECHO) cp ddbserver ./server2/
	$(ECHO) cp ddbserver ./server3/
	cp ../a.db server1/

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
	$(ECHO) $(CXX) -o $@ $^ -lpthread -ltokyocabinet
master: $(MASTEROBJ)
	$(ECHO) $(CXX) -o $@ $^ -ltokyocabinet
	#$(ECHO) mkdir server1 server2 server3
	$(ECHO) cp slave ./server1/
	$(ECHO) cp slave ./server2/
	$(ECHO) cp slave ./server3/
	rm slave
	cp ../a.db server1/

.c.o:
	$(ECHO) $(CXX) -c $(CXXFLAGS) $< -o $@

clean:
	@$(RM) $(CLIENTOBJ) $(SERVEROBJ) $(TARGET) ./server1/ddbserver ./server2/ddbserver ./server3/ddbserver
	@$(RM) -r ./server1/slave ./server2/slave ./server3/slave
	@$(RUBBISHFILES) | xargs $(RM)
	@$(RM) *.o

cleantest:
	@$(RM) $(TESTOBJ) $(TESTTARGET)
	@$(RUBBISHFILES) | xargs $(RM)
