# File: Makefile
# Class: EE 495-Senior Reseach                      # Instructor: Dr. Don Roberts
# Assignment: Location-Dept Cryptosystem            # Date started: 1/4/2018
# Programmer: Kunal Mukherjee                       # Date completed:

client: rawTCPclient.o
		clang -Wall -lmcrypt rawTCPclient.o -o client

rawTCPclient.o: rawTCPclient.c
		clang -Wall -c rawTCPclient.c

clean:
		rm -f *.o *~ client textOut.txt