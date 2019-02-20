#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <time.h>

#define MAX_CHARACTER_SIZE 32
#define MAX_TIME_ANCHOR 64
#define C 300000000
#define TICK 65536 * 975000
#define STDDIV 0


int main(int argc, char const *argv[])
{
	FILE * transmissionFile;
    transmissionFile = fopen(argv[1],"r");

    int anchorNumber = 0;
    int receptionTime[MAX_TIME_ANCHOR] = {'0'};
    uint64_t Tx = 0;
    int receptionTimeindex = 0;

    int dA = atoi(argv[2]);
    int dB = atoi(argv[3]);
    int dC = atoi(argv[4]);

    while(fscanf(transmissionFile, "%d" , &anchorNumber) != EOF)
    {
        fscanf(transmissionFile, "%lu" , &Tx);
        //printf("%d %lu \n",anchorNumber, Tx );

        if (anchorNumber == 0)
        {
            receptionTime[receptionTimeindex] = Tx - 16440 - (dA * C * TICK)  - 16440 - STDDIV;
        }
        else if (anchorNumber == 1)
        {
            receptionTime[receptionTimeindex] = Tx - 16440 - (dB * C * TICK)  - 16440 - STDDIV;
        }
        else if (anchorNumber == 2)
        {
            receptionTime[receptionTimeindex] = Tx - 16440 - (dC * C * TICK)  - 16440 - STDDIV;
        }        
        receptionTimeindex++;
    }

    unsigned char * oBuf = calloc(1, MAX_CHARACTER_SIZE); 
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {        
        oBuf[i] = receptionTime[i];
    }

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
        printf("%02x", oBuf[i]);
    }
    printf("\n");
	return 0;    


}

 
