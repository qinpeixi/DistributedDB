all:	client server
client: dbclient.o dbProtocol.o socketwrapper.o netdbapi.o parseinput.o
	gcc -o $@ $^ 
server: dbserver.o dbProtocol.o socketwrapper.o tcDB.o
	gcc -o $@ $^ -ltokyocabinet
.c.o:
	gcc -c -g $<
clean:
	rm *.o
	rm client server
	rm *.db
	
