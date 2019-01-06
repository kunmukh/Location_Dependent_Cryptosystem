// File: Makefile
// Class: EE 495-Senior Reseach            # Instructor: Mr. Mike Ciholas and Dr. Donald Roberts
// Assignment: Location-Dept Cryptosystem  # Date started: 1/5/2018
//Programmer: Kunal Mukherjee              # Date completed:

//adding the header files
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mcrypt.h> //http://linux.die.net/man/3/mcrypt

#define __USE_BSD /* use bsd'ish ip header */
#include <sys/socket.h> /* these headers are for a Linux system, but */
#include <netinet/in.h> /* the names on other systems are easy to guess.. */
#include <netinet/ip.h>
#define __FAVOR_BSD /* use bsd'ish tcp header */
#include <netinet/tcp.h>
#include <unistd.h>
#include <arpa/inet.h>

//defining the max charc size
#define MAX_CHARACTER_SIZE 32

//displays as well as writes the encrypt file
void printEncryptedTCPPacket(char* ciphertext, int len, FILE* encyptFile)
{
  int v;
  for (v=0; v<len; v++)
  {    
    fprintf(encyptFile, "%d", ciphertext[v]);    
    fprintf(encyptFile, "%s", " ");
    //printf("%d ", ciphertext[v]);
  }  
  
  fprintf(encyptFile, "%s", "\n");
  //printf("\n");
}

//decrypts the AES packets
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

int main(int argc, char const *argv[])
{
  //Step 1: Setting Up socket  
  int fd = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);

  if (fd == -1)
  {
    printf("Socket cannot be opend. Need root permission\n");
    return -1;
  }

  FILE * textOut;
  textOut = fopen("EncrFileOutput.txt","w");

  char buffer[8192]; 

  int counter = 0;

  while (read (fd, buffer, 8192) > 0)
  {   
    if (strncmp( &buffer[ sizeof(struct iphdr) + sizeof(struct tcphdr)],
      "0xABCD", strlen("0xABCD")) == 0)
    {
      printf("TCP Pack Received. Packet Number: %d\n", counter);
      printEncryptedTCPPacket(
        &buffer[ sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen("0xABCD")],
        MAX_CHARACTER_SIZE, textOut);
      counter++;      
    }

    if (strncmp( &buffer[ sizeof(struct iphdr) + sizeof(struct tcphdr)],
      "0xABCE", strlen("0xABCE")) == 0)
    {
      printf("TCP Pack Received.\n");
      printEncryptedTCPPacket(
        &buffer[ sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen("0xABCE")],
         MAX_CHARACTER_SIZE, textOut);  
          
      counter++;      
      printf("TCP Sequence Received. Counter: %d Last Charater: %c\n", counter,
        buffer[ sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen("0xABCD") 
          + MAX_CHARACTER_SIZE - 1]);
      fclose (textOut);
      break; 
    }   
     memset (buffer, 0, 8192);        
  }

    //Step 2: Take the AES encoding and convert it back to digitalized signal
    //decryption algorithm
    printf("Decryption Process Started\n");

    //open the encrypted file to get the IV and the AES blocks 
    //open the output_digitalized text that would be created
    FILE * encyptFileInput;
    encyptFileInput = fopen("EncrFileOutput.txt","r");

    FILE * outputFile;
    outputFile = fopen("DecrOutput.txt","w");  //write protected file      

    //initialize decrypt buffer
    int bufferDecr_len = MAX_CHARACTER_SIZE;
    char * bufferDecr = calloc(1, bufferDecr_len);

    //initialize the key
    char * keyDecr = calloc(1, MAX_CHARACTER_SIZE); //MAX_CHARACTER_SIZE * 8 = 128    
    strncpy(keyDecr, argv[1], MAX_CHARACTER_SIZE);    
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
        fprintf(outputFile, "%s", bufferDecr);       
        bufIndex = 0;
        memset(bufferDecr, 0 , MAX_CHARACTER_SIZE);
        bufferDecr[bufIndex] = AESbuf;
        bufIndex++;
      }        
    }

    if( strlen(bufferDecr) != 0)
    {
      decrypt(bufferDecr, bufferDecr_len, IVDecr, keyDecr, keyDecrsize);
      fprintf(outputFile, "%s", bufferDecr);      
    } 

    printf("Decryption Process Completed\n");

    //closing the output file and free memory    
    fclose(encyptFileInput);
    fclose(outputFile);

    free(keyDecr);
    free(bufferDecr);    
    free(IVDecr);

    //Step 3: Convert the decrypted file to Audio
    printf("\nConvert the decrypted audio sample to audio file\n");

    //opening a output_digitalized file to create the audio
    FILE * outputAudiofile;
    outputAudiofile = popen("ffmpeg -y -f s16le -ar 44100 -ac 1 -i - output.wav -hide_banner", "w");

    FILE * tempAudiosampleFileInput;
    tempAudiosampleFileInput = fopen("DecrOutput.txt","r");

    int16_t audioSampleRead;

    while(fscanf(tempAudiosampleFileInput, "%hd" , &audioSampleRead) != EOF)
    {
        fwrite(&audioSampleRead, 2, 1, outputAudiofile);
    }
     
    // Close input and output pipes       
    pclose(outputAudiofile);    
    fclose(tempAudiosampleFileInput);

    //process completion display
    printf("\n*** Location-Dependent Algorithm Process Completed ***\n\n");

    //play the resultent audio
    FILE * playResultAudio;
    playResultAudio = popen("ffplay -hide_banner -autoexit output.wav", "r");
    pclose (playResultAudio);

    return 0;
}