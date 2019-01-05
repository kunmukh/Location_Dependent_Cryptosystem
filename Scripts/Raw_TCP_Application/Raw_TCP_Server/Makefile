# File: Makefile
# Class: EE 495-Senior Reseach                      # Instructor: Dr. Don Roberts
# Assignment: Location-Dept Cryptosystem            # Date started: 1/4/2018
# Programmer: Kunal Mukherjee                       # Date completed:

server: rawTCPserver.o
		clang -Wall -lmcrypt rawTCPserver.o -o server

rawTCPserver.o: rawTCPserver.c tcp_ip_packet.h
		clang -Wall -c rawTCPserver.c

clean:
		rm -f *.o *~ server DecrOutput.txt tempAud.txt EncrFileOutput.txt output.wav