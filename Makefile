# File: Makefile
# Class: EE 495-Senior Reseach                      # Instructor: Dr. Don Roberts
# Assignment: Location-Dept Cryptosystem            # Date started: 12/26/2018
# Programmer: Kunal Mukherjee                       # Date completed:

server: server.o
		g++ -Wall server.o -o server

server.o: server.c
		g++ -Wall -c server.c

clean:
		rm -f *.o *~ server