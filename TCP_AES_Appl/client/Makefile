# File: Makefile
# Class: EE 495-Senior Reseach                      # Instructor: Dr. Don Roberts
# Assignment: Location-Dept Cryptosystem            # Date started: 3/5/2019
# Programmer: Kunal Mukherjee                       # Date completed:

client: client.o
		clang -Wall -lmcrypt -lcrypto -lssl client.o -o client

client.o: client.c
		clang -Wall -c client.c

clean:
		rm -f *.o *~ client DecrOutput.txt tempAud.txt EncrFileOutput.txt output.wav