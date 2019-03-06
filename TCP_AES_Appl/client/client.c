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

//header file for TCP
#include <sys/socket.h> 
#include <netinet/in.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <inttypes.h>

#define PORT 8080 

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

int tcpServiceRoutine();

void aesToAudioConversion(char const * pass);

void keyFromTxValue(char const * d1, char const * d2, char const * d3);

//cipher text displyer
void printEncryptedFile(char* ciphertext, 
                        int len, 
                        FILE* encyptFile);

void playSong ();

int64_t S64(const char *s);

int main(int argc, char const *argv[])
{
	  //check to see if all the argv is entred
    if (argc != 4)
    {
      printf("Usage: ./client <d1> <d2> <d3>\n");
      return 0;
    }
   
    printf("\n==Location-Dependent Algorithm==\n");

    tcpServiceRoutine();

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

int tcpServiceRoutine()
{
    int sock = 0; 
    struct sockaddr_in serv_addr; 
    char *hello = "Hello from client"; 
    char buffer[1024] = {0}; 

    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

    send(sock , hello , strlen(hello) , 0 );
    printf("Ready to receive\n");

    char * received = "Packet acknowledgement received";

    send(sock , received , strlen(received) , 0 ); 
    int counter = 0;

    uint64_t anchorNumber = 0;
    uint64_t tx = 0;
    uint64_t offset = 0;    

    char txBuff[21]= {0};
    char anchorNumberbuff[21]= {0};
    char offsetBuff[21]= {0};
    char dataBuff[32]= {0};

    int index = 0;

    FILE * receptionFile;
    receptionFile = fopen("receptionTx.dat","w");

    FILE * encrFile;
    encrFile = fopen("EncrFileOutput.txt","w");

    int first = 0;

    while (read( sock , buffer, 1024) > 1)
    {
        printf("\n\nPacket Content: AcNum:%s Tx:%s Offset:%s", &buffer[0], &buffer[50], 
        &buffer[100]);
        printf(" Data: ");     
        for(int i = 0; i < MAX_CHARACTER_SIZE; i++){printf("%d ", buffer[150+i]);}
        printf("\n");
        
        for(int i = 0;  i <= 0 + 21; i++)
            {anchorNumberbuff[index] = buffer[i]; index++;}
        index = 0;        
        for(int i = 50; i <= 50 + 21; i++)
            {txBuff[index] = buffer[i]; index++;}
        index = 0;        
        for (int i = 100; i <= 100 + 21; i++)
            {offsetBuff[index] = buffer[i]; index++;}
        index = 0;
        for (int i = 150; i <= 150 + 32; i++)
            {dataBuff[index] = buffer[i]; index++;}
        index = 0;             

        anchorNumber =  S64(anchorNumberbuff);
        tx = S64(txBuff);
        offset = S64(offsetBuff);        

        printf("Anchor Number: %lu\n", anchorNumber);
        printf("Tx: %lu\n", tx);
        printf("Offset %lu\n", offset);

        if (first < 32)
        {
          fprintf(receptionFile, "%lu ", anchorNumber);
          fprintf(receptionFile, "%lu ",tx);
          fprintf(receptionFile, "%lu\n",offset);

          first++;
        }

        for(int i = 0; i < MAX_CHARACTER_SIZE; i++)
          {fprintf(encrFile, "%d ", buffer[150+i]);}        
        fprintf(encrFile, "%s", "\n");

        send(sock , received , strlen(received) , 0 ); 

        counter++;

        memset(buffer, 0, sizeof(char) * 1024);
        memset(txBuff, 0, sizeof(char) * 21);
        memset(anchorNumberbuff, 0, sizeof(char) * 21);
        memset(offsetBuff, 0, sizeof(char) * 21);
    } 
    
    printf("\n\nNumber of Packet sent: %d\n", counter );

    fclose(receptionFile);
    fclose(encrFile);
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
          //empty scan
          fscanf(receptionFile, "%ld" , &Tx);        
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
  // TBD failed to scan;  
  return 0;  
}

void playSong(void)
{
  //play the resultent audio
  FILE * playResultAudio;
  playResultAudio = popen("ffplay -hide_banner -autoexit output.wav", "r");
  pclose (playResultAudio);
}


