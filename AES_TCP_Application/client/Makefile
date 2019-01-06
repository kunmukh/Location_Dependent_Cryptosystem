# File: Makefile
# Class: EE 495-Senior Reseach                      # Instructor: Dr. Don Roberts
# Assignment: Location-Dept Cryptosystem            # Date started: 1/4/2018
# Programmer: Kunal Mukherjee                       # Date completed:

client: client.o
		clang -Wall -lmcrypt client.o -o client

client.o: client.c
		clang -Wall -c client.c

clean:
		rm -f *.o *~ client