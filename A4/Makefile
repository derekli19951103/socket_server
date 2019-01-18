CC = gcc
PORT = 50540
CFLAGS = -g  -Wall -std=c99 -Werror 
CFLAGS+= -DPORT=\$(PORT)


all: mismatch_server  # removed questions qtree 

mismatch_server: mismatch_server.c mismatch_server.h qtree.o qtree.h 
	$(CC) $(CFLAGS) -o mismatch_server mismatch_server.c qtree.o questions.o 

#categorizer: qtree.o categorizer.c qtree.h
#	$(CC) $(CFLAGS) -o categorizer categorizer.c qtree.o questutils.ions.o

qtree.o: qtree.c qtree.h questions.o 
	$(CC) $(CFLAGS) -c qtree.c 

questions.o: questions.c questions.h
	$(CC) $(CFLAGS) -c questions.c
   
clean:  
	rm qtree.o questions.o mismatch_server
	