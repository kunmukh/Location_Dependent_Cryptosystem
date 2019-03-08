#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <time.h>

#define MAX_CHARACTER_SIZE 32
#define MAX_TIME_ANCHOR 32
#define C 300000000



int main(int argc, char const *argv[])
{
	FILE * transmissionFile;
    transmissionFile = fopen(argv[1],"r");

    int anchorNumber = 0;
    int receptionTime[MAX_TIME_ANCHOR] = {'0'};
    uint64_t Trx = 0;
    int receptionTimeindex = 0;    

    uint64_t TbtwnOffset = 0.0025 * 975000 * 65536;
    uint64_t Trxlast = 0;
    uint64_t Tslot = 0.0000000033 * 975000 * 65536; //time width of each value of key 3.3nsec
                                                   //210 NT ticks    

    while(fscanf(transmissionFile, "%d" , &anchorNumber) != EOF)
    {
        fscanf(transmissionFile, "%lu" , &Trx);        
        
        receptionTime[receptionTimeindex] = (Trx - Trxlast - TbtwnOffset) / Tslot;
        Trxlast = Trx;        
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

 
