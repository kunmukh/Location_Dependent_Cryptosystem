// File: Makefile
// Class: EE 495-Senior Reseach            # Instructor: Mr. Mike Ciholas and Dr. Donald Roberts
// Assignment: Location-Dept Cryptosystem  # Date started: 3/5/2018
//Programmer: Kunal Mukherjee              # Date completed:

//adding the header files for AES
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mcrypt.h> //http://linux.die.net/man/3/mcrypt

//header file for Tx Maker
#include <openssl/sha.h>
#include <time.h>

//defining the max charc size
#define MAX_CHARACTER_SIZE 32
#define MAX_TIME_ANCHOR 32
#define C 300000000
#define PACKET_LENGTH 1984


//the decrypt function
int decrypt(
    void* buffer,
    int buffer_len,
    char* IV, 
    char* key,
    int key_len 
);

void aesToAudioConversion(char const * pass);

void keyFromTxValue(char * password);

//cipher text displyer
void printEncryptedFile(char* ciphertext, 
                        int len, 
                        FILE* encyptFile);

void playSong ();

int64_t S64(const char *s);

int main(int argc, char const *argv[])
{
    //check to see if all the argv is entred
    if (argc != 1)
    {
      printf("Usage: ./client\n");
      return 0;
    }
    //local variable password storage
    char * password = calloc(1, SHA256_DIGEST_LENGTH);
   
    printf("\n==Location-Dependent Algorithm==\n"); 

    //receptionTx.dat          

    keyFromTxValue(password);    

    aesToAudioConversion(password);
   
    printf("\n*** Location-Dependent Algorithm Process Completed ***\n\n");

    playSong();
   
    free(password);
    return 0;
} 

void keyFromTxValue(char * password)
{
    printf("\nThe Key extraction Process Started\n");

    FILE * transmissionFile;
    transmissionFile = fopen("receptionTx.dat","r");

    int anchorNumber = 0;
    int receptionTime[MAX_TIME_ANCHOR] = {'0'};
    uint64_t Trx = 0, firstByteNumber = 0;
    int receptionTimeindex = 0;    

    uint64_t TbtwnOffset = 0.0025 * 975000 * 65536;
    uint64_t Trxlast = 0;
    uint64_t Tslot = 0.00000000667 * 975000 * 65536; //time width of each value of 
                                                   //key 6.67nsec 426 NT ticks  
    uint64_t Tfirst = 0; 

    FILE * debugFile;
    debugFile = fopen("Debug.txt","w");
    int bufferNum = 0;

    fscanf(transmissionFile, "%lu" , &Tfirst);
    while(fscanf(transmissionFile, "%d" , &anchorNumber) != EOF)
    {        
        fscanf(transmissionFile, "%lu" , &Trx);  
        fscanf(transmissionFile, "%lu" , &firstByteNumber);       
        
        if(bufferNum < 32)
        {
          receptionTime[receptionTimeindex] = 
                        (Trx - Trxlast - TbtwnOffset) / Tslot; 
        }

        fprintf(debugFile, "Ach: %-5d ", anchorNumber); 
        fprintf(debugFile, "Tslot: %-5lu ", ((Trx - Trxlast - TbtwnOffset) / Tslot));
        //fprintf(debugFile, "Tnoise: %-5d ", (int)Tnoise);
        fprintf(debugFile, "Trx+n: %-15lu ", (Trx));
        fprintf(debugFile, "Trxlast: %-15lu ", Trxlast);
        fprintf(debugFile, "TbtwnOffset: %-15lu ", TbtwnOffset);       
        fprintf(debugFile, "IstB %-10lu\n", firstByteNumber);

        Trxlast = Trx;
        receptionTimeindex++;
        bufferNum++;
    }    

    printf("The Key Is: \n");
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
       
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
      password[i] = oBuf[i];;
    }   
    
    fclose(transmissionFile);    
    fclose(debugFile);
    printf("\nThe Key extraction Process Ended\n");               
}

void aesToAudioConversion(char const * password)
{  
  //decryption algorithm
  printf("Decryption Process Started\n");
  printf("Converting the decrypted audio sample to audio file\n");

  //open the encrypted file to get the IV and the AES blocks 
  //open the output_digitalized text that would be created
  FILE * encyptFileInput;
  encyptFileInput = fopen("EncrFileOutput.txt","r");

  //initialize decrypt buffer
  int bufferDecr_len = MAX_CHARACTER_SIZE; 
  char * bufferDecr = calloc(1, bufferDecr_len);

  //initialize the key
  char * keyDecr = calloc(1, MAX_CHARACTER_SIZE); //MAX_CHARACTER_SIZE * 8 = 128    
  strncpy(keyDecr, password, MAX_CHARACTER_SIZE);    
  int keyDecrsize = MAX_CHARACTER_SIZE;
  
  //getting IV
  char * IVDecr = calloc(1, MAX_CHARACTER_SIZE);
  int IVBuf = 0;

  for (int i = 0; i < MAX_CHARACTER_SIZE; i++)
  {
    fscanf(encyptFileInput, "%d" , &IVBuf);
    IVDecr[i] = IVBuf;
  }

  //display the decrypting info
  printf("The IV is: %s\n", IVDecr);
  printf("The Key is: %s\n", keyDecr);

  //initilizing he tempAESbuffer and bufferIndex
  int  AESbuf = 0;
  int bufIndex = 0;

  //opening a output_digitalized file to create the audio
  FILE * outputAudiofile;
  outputAudiofile = popen("ffmpeg -y -f s16le -ar 44100 -ac 1 -i - output.wav -hide_banner", "w");

  //getting the AES blocks from the encrypt_file and decrypting it
  while(fscanf(encyptFileInput, "%d" , &AESbuf) != EOF)
  {        
    if(bufIndex < MAX_CHARACTER_SIZE)
    {
      bufferDecr[bufIndex] = AESbuf;
      bufIndex++;  
    }
    else
    {             
      decrypt(bufferDecr, bufferDecr_len, IVDecr, keyDecr, keyDecrsize);
      fwrite(bufferDecr, 4, 1, outputAudiofile);

      bufIndex = 0;
      memset(bufferDecr, 0 , MAX_CHARACTER_SIZE);
      bufferDecr[bufIndex] = AESbuf;
      bufIndex++;
    }        
  }

  if( strlen(bufferDecr) != 0)
  {
    decrypt(bufferDecr, bufferDecr_len, IVDecr, keyDecr, keyDecrsize);        
    fwrite(bufferDecr, 4, 1, outputAudiofile);
  } 

  printf("Decryption Process Completed and Audio File Made\n");

  //closing the output file and free memory    
  fclose(encyptFileInput);   

  free(keyDecr);
  free(bufferDecr);    
  free(IVDecr); 

  pclose(outputAudiofile); 
}

int decrypt(void* buffer, int buffer_len, char* IV, char* key, int key_len)
{
  MCRYPT td = mcrypt_module_open("rijndael-256", NULL, "cbc", NULL);
  int blocksize = mcrypt_enc_get_block_size(td);
  if( buffer_len % blocksize != 0 )
    {
      return 1;
    }
  
  mcrypt_generic_init(td, key, key_len, IV);
  mdecrypt_generic(td, buffer, buffer_len);
  mcrypt_generic_deinit (td);
  mcrypt_module_close(td);
  
  return 0;
}

int64_t S64(const char *s) 
{
  int64_t i;
  char c ;
  int scanned = sscanf(s, "%"  PRIu64 "%c", &i, &c);
  if (scanned == 1) return i;
  if (scanned > 1) {
    // TBD about extra data found
    return i;
    }
  // TBD has failed to scan;  
  return 0;  
}

void playSong(void)
{
  //play the resultent audio
  FILE * playResultAudio;
  playResultAudio = popen("ffplay -hide_banner -autoexit output.wav", "r");
  pclose (playResultAudio);
}


