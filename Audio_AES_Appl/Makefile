# File: Makefile
# Class: EE 495-Senior Reseach                      # Instructor: Dr. Don Roberts
# Assignment: Location-Dept Cryptosystem            # Date started: 12/26/2018
# Programmer: Kunal Mukherjee                       # Date completed:

server: server.o
		clang -Wall -lmcrypt server.o -o server

server.o: server.c
		clang -Wall -c server.c

clean:
		rm -f *.o *~ server DecrOutput.txt tempAud.txt EncrFileOutput.txt output.wav