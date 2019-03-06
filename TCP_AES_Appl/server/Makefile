# File: Makefile
# Class: EE 495-Senior Reseach                      # Instructor: Dr. Don Roberts
# Assignment: Location-Dept Cryptosystem            # Date started: 3/5/2019
# Programmer: Kunal Mukherjee                       # Date completed:

server: server.o
		clang -Wall -lmcrypt -lcrypto -lssl server.o -o server

server.o: server.c
		clang -Wall -c server.c

clean:
		rm -f *.o *~ server DecrOutput.txt tempAud.txt EncrFileOutput.txt output.wav transmission.dat