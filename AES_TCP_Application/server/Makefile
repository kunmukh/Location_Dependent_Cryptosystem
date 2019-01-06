# File: Makefile
# Class: EE 495-Senior Reseach                      # Instructor: Dr. Don Roberts
# Assignment: Location-Dept Cryptosystem            # Date started: 1/5/2018
# Programmer: Kunal Mukherjee                       # Date completed:

server: server.o
		clang -Wall -lmcrypt server.o -o server

server.o: server.c tcp_ip_packet.h
		clang -Wall -c server.c

clean:
		rm -f *.o *~ server 