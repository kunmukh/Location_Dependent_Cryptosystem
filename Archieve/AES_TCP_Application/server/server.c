// File: Makefile
// Class: EE 495-Senior Reseach            # Instructor: Mr. Mike Ciholas and Dr. Donald Roberts
// Assignment: Location-Dept Cryptosystem  # Date started: 1/5/2018
//Programmer: Kunal Mukherjee              # Date completed:

//adding the header files for cryptosystem
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mcrypt.h> //http://linux.die.net/man/3/mcrypt

//header files for raw TCP
#define __USE_BSD /* use bsd'ish ip header */
#include <sys/socket.h> /* these headers are for a Linux system, but */
#include <netinet/in.h> /* the names on other systems are easy to guess.. */
#include <netinet/ip.h>
#define __FAVOR_BSD /* use bsd'ish tcp header */
#include <netinet/tcp.h>
#include <unistd.h>
#include <arpa/inet.h>

//header file that creates a new packet
#include "tcp_ip_packet.h"

//defining the max charc size
#define MAX_CHARACTER_SIZE 32
#define P 25   

//encryption process
int encrypt(void* buffer, int buffer_len, char* IV, char* key, int key_len)
{
  MCRYPT td = mcrypt_module_open("rijndael-256", NULL, "cbc", NULL);
  int blocksize = mcrypt_enc_get_block_size(td);
  if( buffer_len % blocksize != 0 )
    {
      return 1;
    }

  mcrypt_generic_init(td, key, key_len, IV);
  mcrypt_generic(td, buffer, buffer_len);
  mcrypt_generic_deinit (td);
  mcrypt_module_close(td);
  
  return 0;
}

//displays as well as writes the encrypt file
void printEncryptedFile(char* ciphertext, int len, FILE* encyptFile)
{
  int v;
  for (v=0; v<len; v++)
  {    
    fprintf(encyptFile, "%d", ciphertext[v]);
    fprintf(encyptFile, "%s", " ");
  }  
  
  fprintf(encyptFile, "%s", "\n");
}

void setHDRINCL(int s)
{
    int one = 1;
    const int *val = &one;
    if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    {     
      printf ("Warning: Cannot set HDRINCL!\n");
    }
}

int main(int argc, char const *argv[])
{
	  //check to see if all the argv is entred
    /*if (argc != 2)
    {
      printf("Usage: ./server <password> \n");
      return 0;
    }  

    //Step 1: Take Audio file -> Digitalize the signal
    printf("==Location-Dependent Algorithm==\n\n");

    //open the respective audio and audio_text input files
    FILE * inputAudiofile;   
    inputAudiofile  = popen("ffmpeg -i input.wav -hide_banner -f s16le -ac 1 -", "r");    

    FILE * tempAudiosampleFile;
    tempAudiosampleFile = fopen("tempAud.txt","w");
     
    // Read, modify and write one sample at a time
    int16_t audioSample;        

    while(fread(&audioSample, 2, 1, inputAudiofile)) // read one 2-byte sample
    {   
        fprintf(tempAudiosampleFile, "%hd ", audioSample);       
    }
 
    //closing the pipe and file
    pclose(inputAudiofile); 
    fclose(tempAudiosampleFile);

    printf("\nAudio file extractd and audio sample file created.\n");

    //Step 2: Take the digitalized signal and encode with AES
    printf("\nEncryption Process Started\n");

    //open the audio_text input and the encrypted file
    FILE * inputFile;
    inputFile = fopen("tempAud.txt","r"); 

    FILE * encyptFileOutput;
    encyptFileOutput = fopen("EncrFileOutput.txt","w");

    //create a MCRYPT to get certain info
    MCRYPT td = mcrypt_module_open("rijndael-256", NULL, "cbc", NULL);
	  
	  //A random block should be placed as the first block (IV) 
	  //so the same block or messages always encrypt to something different.
	  char * IVEncr = malloc(mcrypt_enc_get_iv_size(td)); //return 8
	  FILE * fp;
    fp = fopen("/dev/urandom", "r");
    fread(IVEncr, 1, mcrypt_enc_get_iv_size(td), fp);
    fclose(fp);
    mcrypt_generic_end(td);	  
    //place the IV in the encrypted file
    printEncryptedFile(IVEncr , mcrypt_enc_get_iv_size(td) , encyptFileOutput);

    //check to see if the key is MAX_CHARACTER_SIZE charcter long
    char * keyEncr = calloc(1, MAX_CHARACTER_SIZE); //MAX_CHARACTER_SIZE * 8 = 128    
    strncpy(keyEncr, argv[1], MAX_CHARACTER_SIZE);	  
	  int keyEncrsize = MAX_CHARACTER_SIZE; // 256 bits 

    //initialize the buffer
	  int bufferEncr_len = MAX_CHARACTER_SIZE;    
    char * bufferEncr = calloc(1, bufferEncr_len); 
	  
    //Encryption algorithm info display  
    printf("The IV is: %s\n", IVEncr);
    printf("The Key is: %s\n", keyEncr);

    while(fgets(bufferEncr, sizeof bufferEncr, inputFile) != NULL)
    {
      //process buffer           
      encrypt(bufferEncr, bufferEncr_len, IVEncr, keyEncr, keyEncrsize);            
      printEncryptedFile(bufferEncr , bufferEncr_len , encyptFileOutput);          
    }
    if (feof(inputFile)) 
    {
      // hit end of file     
      printf("Encryption Process Completed\n\n");
    }

    //closing file and free memory after encryption
    fclose(inputFile);
    fclose(encyptFileOutput);

    free(IVEncr);
    free(bufferEncr);
    free(keyEncr);*/

    //Step 3: TCP socket setting and sendign it  
    int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);  // open raw socket 

    struct sockaddr_in sin;
    sin.sin_family = AF_INET;
    sin.sin_port = htons (P);
    sin.sin_addr.s_addr = inet_addr ("127.0.0.1"); 
      
    char datagram[4096];   
    memset (datagram, 0, 4096);

    char * code;
    char data  [32] = {"0"};  

    code = "0xABCD";   

    FILE * textin;
    textin = fopen("dummy.txt","r"); //EncrFileOutput.txt

    int AESbuf = 0, bufIndex = 0, counter = 0;       

    int fdR = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);
    char bufferR[8192]; 

    while(fscanf(textin, "%d" , &AESbuf) != EOF)
    {        
        if(bufIndex < MAX_CHARACTER_SIZE)
          {
              //printf("%d AES: %d\n",bufIndex, AESbuf);
              data[bufIndex] = AESbuf;
              bufIndex++;              
          }

        else
          {            
              setHDRINCL(s);               

              unsigned short int ip_len; 
              char * newDatagram = createDatagram(datagram, P, code, data, sin, &ip_len);

              if (sendto (s, newDatagram, ip_len, 0, (struct sockaddr *) &sin, sizeof (sin)) < 0)    
              {
                  printf ("error\n");
              }  
              else
              {
                  printf("Waiting for reception signal\n");

                  while (read (fdR, bufferR, 8192) > 0)
                  {   
                    if (strncmp( &bufferR[ sizeof(struct iphdr) + sizeof(struct tcphdr)],
                      "0xABCR", strlen("0xABCR")) == 0)
                    {
                      break;
                    }
                                        
                  }

                  printf ("send Packtet Num: %d \n", counter); 
                  counter++;                                          
              }

              bufIndex = 0;
              memset (datagram, 0, MAX_CHARACTER_SIZE);
              memset (data, 0, MAX_CHARACTER_SIZE);              
              
              //printf("%d AES: %d\n",bufIndex, AESbuf);
              data[bufIndex] = AESbuf;
              bufIndex++; 

              usleep(2);             

            }         
      }

    setHDRINCL(s);  

    code = "0xABCE";

    unsigned short int ip_len; 
    char * newDatagram = createDatagram(datagram, P, code, data, sin, &ip_len);

    if (sendto (s, newDatagram, ip_len, 0, (struct sockaddr *) &sin, sizeof (sin)) < 0)    
    {
      printf ("error\n");
    }  
    else
    {
        counter++; 
        printf ("send Terminating Sequence. Packet Num: %d Last Char: %c\n", counter,
        data[MAX_CHARACTER_SIZE -1]);                 
    }

    fclose(textin);

    return 0;
}