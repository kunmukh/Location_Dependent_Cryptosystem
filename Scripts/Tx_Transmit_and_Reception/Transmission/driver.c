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

    //create the Tx array
    int oTx [MAX_CHARACTER_SIZE] = {'0'};
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
        oTx[i] = obuf[i];
        //printf("%d\n", oTx[i]);
    }
    
    int dA = atoi(argv[2]);    
    int dB = atoi(argv[3]);    
    int dC = atoi(argv[4]);

    srand(time(0));

    //Timing Variables
    uint64_t Tnet = 97500 * 65536;
    uint64_t TstartOffset = 0.005 * 975000 * 65536; //time for all the net pakt to go 5ms
    											   //319488000 (NT ticks)
    uint64_t TbtwnOffset = 0.0025 * 975000 * 65536; //time between each of my seg
    											   //159744000 (NT ticks)
    uint64_t Tslot = 0.0000000033 * 975000 * 65536; //time width of each value of key 3.3nsec
    											   //210 NT ticks
    uint64_t TdistA = (dA/C) * 975000 * 65536 ;
    uint64_t TdistB = (dB/C) * 975000 * 65536;
    uint64_t TdistC = (dC/C) * 975000 * 65536;

    uint64_t Tdistlast = 0;
    uint64_t Ttxlast = 0;

    uint64_t Tx = 0;
    uint64_t Txfirst = 0;

    //first transmission being sent    

    int Slot = 0; 
    printf("The Sequence of Transmission\n");

    FILE * transmissionFile;
    transmissionFile = fopen("transmission.dat","w");

    //first transmission
    Txfirst = Tnet + TstartOffset;
    printf("%lu\n", Txfirst);

     for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {    	   	
    	Slot = oTx[i];
    	if (rand() % 2 == 0)
    	{    		
    		Tx = Ttxlast + Tdistlast + TbtwnOffset - TdistA + (Slot + 0.5) * Tslot; 
    		Tdistlast = TdistA;
    		printf("Slot: %d Anchor A: %lu \n", oTx[i], Tx);
    		fprintf(transmissionFile, "%d ", 0);     		
    		
    	}
    	else if (rand() % 3 == 0)
    	{
    		Tx = Ttxlast + Tdistlast + TbtwnOffset - TdistB + (Slot + 0.5) * Tslot;
    		Tdistlast = TdistB; 
    		printf("Slot: %d Anchor A: %lu \n", oTx[i], Tx);
    		fprintf(transmissionFile, "%d ", 1); 
    	}
    	else 
    	{
    		Tx = Ttxlast + Tdistlast + TbtwnOffset - TdistC + (Slot + 0.5) * Tslot; 
    		Tdistlast = TdistC;
    		printf("Slot: %d Anchor A: %lu \n", oTx[i], Tx);
    		fprintf(transmissionFile, "%d ", 2); 
    	}
    	  		
    	fprintf(transmissionFile, "%lu\n",Tx);
    	Ttxlast = Tx;    	
    }

	return 0;    


}

 
