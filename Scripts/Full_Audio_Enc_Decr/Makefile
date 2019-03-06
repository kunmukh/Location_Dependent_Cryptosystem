# File: Makefile
# Class: EE 495-Senior Reseach                      # Instructor: Dr. Don Roberts
# Assignment: Location-Dept Cryptosystem            # Date started: 2/5/2018
# Programmer: Kunal Mukherjee                       # Date completed:

driver: driver.o
		clang -Wall -lcrypto -lmcrypt -lssl driver.o -o driver

driver.o: driver.c
		clang -Wall -c driver.c

clean:
		rm -f *.o *~ driver