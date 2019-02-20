#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <time.h>

#define MAX_CHARACTER_SIZE 32
#define MAX_TIME_ANCHOR 64
#define C 300000000
#define TICK 65536 * 975000
#define OFFSET 0

int main(int argc, char const *argv[])
{
	//input buffer
	char * ibuf = calloc(1, MAX_CHARACTER_SIZE);  
    strncpy(ibuf, argv[1], MAX_CHARACTER_SIZE);

    printf("Password: %s\n", ibuf);
	//output buffer
    unsigned char *obuf = SHA256(ibuf, strlen(ibuf), 0);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
        printf("%02x", obuf[i]);
    }
    printf("\n");    

    //Tx = 16440 + ((d * c * TICK) + 16440 + (NOISE / c) * TICK
    //TICK = 65536 tick / sec * 975000
    //NOISE = 0.003 m
    //d = 10 m
    //c = 300000000 m / sec

    //create the Tx array
    int oTx [MAX_CHARACTER_SIZE] = {'0'};
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
        oTx[i] = obuf[i];
        //printf("%d\n", oTx[i]);
    }

    uint64_t anchorA[MAX_TIME_ANCHOR] = {'0'};
    int indexA = 0, dA = atoi(argv[2]);
    uint64_t anchorB[MAX_TIME_ANCHOR] = {'0'};
    int indexB = 0, dB = atoi(argv[3]);
    uint64_t anchorC[MAX_TIME_ANCHOR] = {'0'};
    int indexC = 0, dC = atoi(argv[4]);
    int anchorChoice[MAX_TIME_ANCHOR]  = {'0'};
    int indexChoice = 0;

    srand(time(0));

    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
    	if(rand() % 2 == 0)
    	{
    		anchorA[indexA] = oTx[i] + 16440 + (dA * C * TICK ) + 16440 + OFFSET;
    		indexA++;
    		anchorChoice[indexChoice] = 0;
    		indexChoice++;
    	}else if (rand() % 3 == 0)
    	{
    		anchorB[indexB] = oTx[i] + 16440 + (dB * C * TICK)  + 16440 + OFFSET;
    		indexB++;
    		anchorChoice[indexChoice] = 1;
    		indexChoice++;
    	}else
    	{
    		anchorC[indexC] = oTx[i] + 16440 + (dC * C * TICK)  + 16440 + OFFSET;
    		indexC++;
    		anchorChoice[indexChoice] = 2;
    		indexChoice++;
    	}
    }

    printf("\nAnchor A: \n");
    for (int i = 0; i < indexA; i++) 
    {
        printf("Num %d %lu \n",i+1, anchorA[i]);
    }
    printf("\n");

    printf("Anchor B: \n");
    for (int i = 0; i < indexB; i++) 
    {
        printf("Num %d %lu \n",i+1, anchorB[i]);
    }
    printf("\n");

    printf("Anchor C: \n");
    for (int i = 0; i < indexC; i++) 
    {
        printf("Num %d %lu \n",i+1, anchorC[i]);
    }
    printf("\n");

    int anchorATransmit = 0, anchorBTransmit = 0, anchorCTransmit = 0; 
    printf("The Sequence of Transmission\n");

    FILE * transmissionFile;
    transmissionFile = fopen("transmission.dat","w");

     for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
    	printf("Transmission %0d ", i );
    	fprintf(transmissionFile, "%d ", anchorChoice[i]);
    	if (anchorChoice[i] == 0)
    	{
    		printf("Anchor A: %lu \n", anchorA[anchorATransmit]);    		
    		fprintf(transmissionFile, "%lu\n", anchorA[anchorATransmit]);
    		anchorATransmit++;
    	}
    	if (anchorChoice[i] == 1)
    	{
    		printf("Anchor B: %lu \n", anchorB[anchorBTransmit]);
    		fprintf(transmissionFile, "%lu\n", anchorB[anchorBTransmit]);
    		anchorBTransmit++;
    	}
    	if (anchorChoice[i] == 2)
    	{
    		printf("Anchor C: %lu \n", anchorC[anchorCTransmit]);
    		fprintf(transmissionFile, "%lu\n", anchorC[anchorCTransmit]);
    		anchorCTransmit++;
    	}
    }

	return 0;    


}

 
