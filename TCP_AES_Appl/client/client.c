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
#define MAX_TIME_ANCHOR 64
#define C 300000000
#define TICK 65536 * 975000
#define STDDIV 0

//the decrypt function
int decrypt(
    void* buffer,
    int buffer_len,
    char* IV, 
    char* key,
    int key_len 
);

void  aesToAudioConversion(char const * pass);

void keyFromTxValue(char const * d1, char const * d2, char const * d3);

//cipher text displyer
void printEncryptedFile(char* ciphertext, 
                        int len, 
                        FILE* encyptFile);

void playSong ();

int main(int argc, char const *argv[])
{
	  //check to see if all the argv is entred
    if (argc != 4)
    {
      printf("Usage: ./client <d1> <d2> <d3>\n");
      return 0;
    }
   
    printf("\n==Location-Dependent Algorithm==\n");

    //tcp
    keyFromTxValue(argv[1], argv[2], argv[3]);

    //*******
    char * password = calloc(1, SHA256_DIGEST_LENGTH);
    FILE * keyFile;
    keyFile = fopen("key.txt","r");   
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
      fscanf(keyFile, "%c", &password[i]);      
    }    
    fclose(keyFile);    
    ///*****

    aesToAudioConversion(password);
   
    printf("\n*** Location-Dependent Algorithm Process Completed ***\n\n");

    playSong();
   
    free(password);
    return 0;
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

void keyFromTxValue(char const * d1, char const * d2, char const * d3)
{
      printf("\nThe Key extraction Process Started\n");

      FILE * receptionFile;
      receptionFile = fopen("receptionTx.dat","r");

      int anchorNumber = 0;
      int receptionTime[MAX_TIME_ANCHOR] = {'0'};
      uint64_t Tx = 0;
      int receptionTimeindex = 0;

      int dA = atoi(d1);
      int dB = atoi(d2);
      int dC = atoi(d3);

      while(fscanf(receptionFile, "%d" , &anchorNumber) != EOF)
      {
          fscanf(receptionFile, "%lu" , &Tx);
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

      FILE * keyFile;
      keyFile = fopen("key.txt","w");
      char * password = calloc(1, SHA256_DIGEST_LENGTH);   
      for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
      {
        password[i] = oBuf[i];;
      }   
      for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
      {
          fprintf(keyFile, "%c", password[i]);          
      }
      fprintf(keyFile, "%s","\n");
      fclose(receptionFile);
      fclose(keyFile);

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

void playSong(void)
{
  //play the resultent audio
  FILE * playResultAudio;
  playResultAudio = popen("ffplay -hide_banner -autoexit output.wav", "r");
  pclose (playResultAudio);
}


