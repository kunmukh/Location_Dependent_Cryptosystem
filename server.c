// File: Makefile
// Class: EE 495-Senior Reseach                      # Instructor: Dr. Don Roberts
// Assignment: Location-Dept Cryptosystem            # Date started: 12/26/2018
//Programmer: Kunal Mukherjee                       # Date completed:

//adding the header files
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <mcrypt.h> //http://linux.die.net/man/3/mcrypt

//the encrypt function
int encrypt(
    void* buffer,
    int buffer_len, /* Because the plaintext could include null bytes*/
    char* IV, 
    char* key,
    int key_len 
);

int main(int argc, char const *argv[])
{
	/* code */
	printf("This is a test\n");
	return 0;
}	