testDB: testDB.o tcDB.o
	gcc -o $@ $^ -ltokyocabinet
.c.o:
	gcc -c -g $<
clean:
	rm *.o
	rm *.db
	rm testDB
