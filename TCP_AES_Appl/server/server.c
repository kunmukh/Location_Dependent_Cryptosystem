// File: Makefile
// Class: EE 495-Senior Reseach            # Instructor: Mr. Mike Ciholas and Dr. Donald Roberts
// Assignment: Location-Dept Cryptosystem  # Date started: 3/5/2018
//Programmer: Kunal Mukherjee              # Date completed:

//adding the header files for encryption
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <mcrypt.h> //http://linux.die.net/man/3/mcrypt

//header for Tx values
#include <openssl/sha.h>
#include <time.h>

//header for TCP tranfer
#include <unistd.h> 
#include <sys/socket.h> 
#include <netinet/in.h>  
#include <inttypes.h>

#define PORT 8080 

//defining the max charc size
#define MAX_CHARACTER_SIZE 32
#define MAX_TIME_ANCHOR 64
#define C 300000000
#define TICK 65536 * 975000
#define OFFSET 0

//the encrypt function
int encrypt(
    void* buffer,
    int buffer_len, /* Because the plaintext could include null bytes*/
    char* IV, //initilization vector
    char* key,
    int key_len 
);

//cipher text displyer
void printEncryptedFile(char* ciphertext, 
                        int len, 
                        FILE* encyptFile);

//Audio->AES
void audioToAESConversion(char const * pass);

//create the Tx values
void txValueFromKey(char const * key, char const * d1, char const * d2, char const * d3);

//sends the TCP value
void TCPServiceRoutine();


int main(int argc, char const *argv[])
{
	  //check to see if all the argv is entred
    if (argc != 5)
    {
      printf("Usage: ./server <password> <d1> <d2> <d3>\n");
      return 0;
    }  

    //*****
    unsigned char *obuf = SHA256(argv[1], strlen(argv[1]), 0);
    char * password = calloc(1, SHA256_DIGEST_LENGTH);   
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
      password[i] = obuf[i];            
    } 
    //*****

    audioToAESConversion(password);

    txValueFromKey(argv[1], argv[2], argv[3], argv[4]);

    //TCPServiceRoutine();

    return 0;
}	

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

//the function that creates the AUDIO->AES
void audioToAESConversion(char const * pass)
{
    //Step 1: Take Audio file -> Encypt the signal
    printf("\n==Location-Dependent Algorithm==\n");
    printf("\nEncryption Process Started\n");

    //open the respective audio and audio_text input files
    FILE * inputAudiofile;   
    inputAudiofile  = popen("ffmpeg -i input.wav -hide_banner -f s16le -ac 1 -", "r");    
    
    //setup AES Encrypt parameter
    //open the audio_text input and the encrypted file
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
    strncpy(keyEncr, pass, MAX_CHARACTER_SIZE);    
    int keyEncrsize = MAX_CHARACTER_SIZE; /* 256 bits */

    //initialize the buffer
    int bufferEncr_len = MAX_CHARACTER_SIZE;    
    char * bufferEncr = calloc(1, bufferEncr_len); 
    
    //Encryption algorithm info display  
    printf("The IV is: %s\n", IVEncr);
    printf("The Key is: %s\n\n", keyEncr);        

    while(fread(bufferEncr, 4, 1, inputAudiofile)) // read one 4-byte sample
    {   
      encrypt(bufferEncr, bufferEncr_len, IVEncr, keyEncr, keyEncrsize);            
      printEncryptedFile(bufferEncr , bufferEncr_len , encyptFileOutput);       
    }
    printf("Encryption Process Completed\n\n");

    //closing the pipe and file and freeing memory space
    pclose(inputAudiofile);
    fclose(encyptFileOutput);
    free(IVEncr);
    free(bufferEncr);
    free(keyEncr);
}

//the function that creates the Tx values
void txValueFromKey(char const * key,char const * d1, char const * d2, char const * d3)
{
    printf("\n Tx Value Creation starts \n");

    //input buffer
    char * ibuf = calloc(1, MAX_CHARACTER_SIZE);  
    strncpy(ibuf, key, MAX_CHARACTER_SIZE); 

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
    int indexA = 0, dA = atoi(d1);
    uint64_t anchorB[MAX_TIME_ANCHOR] = {'0'};
    int indexB = 0, dB = atoi(d2);
    uint64_t anchorC[MAX_TIME_ANCHOR] = {'0'};
    int indexC = 0, dC = atoi(d3);
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

    fclose(transmissionFile);

    printf("\nTx value have been created\n");
}

//the function sends the TCP packets
void TCPServiceRoutine()
{
  printf("\nReady to Transmit Data.\n");
  int server_fd, new_socket, valread; 
  struct sockaddr_in address; 
  int opt = 1; 
  int addrlen = sizeof(address); 
  char buffer[1024] = {0}; 
  
  // Creating socket file descriptor 
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
  { 
      perror("socket failed"); 
      exit(EXIT_FAILURE); 
  } 
  
  // Forcefully attaching socket to the port 8080 
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                              &opt, sizeof(opt))) 
  { 
      perror("setsockopt"); 
      exit(EXIT_FAILURE); 
  } 
  address.sin_family = AF_INET; 
  address.sin_addr.s_addr = INADDR_ANY; 
  address.sin_port = htons( PORT ); 
  
  // Forcefully attaching socket to the port 8080 
  if (bind(server_fd, (struct sockaddr *)&address, 
                              sizeof(address))<0) 
  { 
      perror("bind failed"); 
      exit(EXIT_FAILURE); 
  } 
  if (listen(server_fd, 3) < 0) 
  { 
      perror("listen"); 
      exit(EXIT_FAILURE); 
  } 
  if ((new_socket = accept(server_fd, (struct sockaddr *)&address, 
                  (socklen_t*)&addrlen))<0) 
  { 
      perror("accept"); 
      exit(EXIT_FAILURE); 
  } 

  valread = read( new_socket , buffer, 1024);     
  printf("%s\n",buffer );    

  int counter = 0;

  char packet [250] = {0};
  memset(packet, 0, sizeof(char) * 250);
  FILE * txFile;
  txFile = fopen("transmission.dat","r");
  FILE * dataFile;
  dataFile = fopen("EncrFileOutput.txt","r");

  uint64_t anchorNumber = 0;
  uint64_t Tx = 0;
  uint64_t offset = OFFSET;
  int data = 0;

  char txBuff[21]= {0};
  char anchorNumberbuff[21]= {0};
  char offsetBuff[21]= {0};
  char dataBuff[32]={0};

  int breakflag = 0;

  while (read( new_socket , buffer, 1024) > 1)
  {        
      memset(packet, 0, sizeof(char) * 250);

      printf("%s\n",buffer);

      for (int i = 0; i < MAX_CHARACTER_SIZE; i++)
      {
        if(fscanf(dataFile, "%d", &data) != EOF)
        {          
          dataBuff[i] = data;
        }
        else
        {
          breakflag = 1;
          break;
        }       
        
      }           

      if (breakflag == 1)
      {
        break;
      }

      if (fscanf(txFile, "%lu" , &anchorNumber) == EOF)
      {
        fseek(txFile, 0, SEEK_SET);
      }
      else
      {
        sprintf(anchorNumberbuff, "%" PRIu64, anchorNumber);

        fscanf(txFile, "%lu" , &Tx);
        sprintf(txBuff, "%" PRIu64, Tx);
      }  

      sprintf(offsetBuff, "%" PRIu64, offset);        

      memcpy(&packet[0], anchorNumberbuff, sizeof(anchorNumberbuff));
      memcpy(&packet[50], txBuff, sizeof(txBuff));
      memcpy(&packet[100], offsetBuff, sizeof(offsetBuff));
      memcpy(&packet[150], dataBuff, sizeof(dataBuff));     

      printf("\n\nPacket Content: AcNum:%s Tx:%s Offset:%s", &packet[0], &packet[50], 
        &packet[100]);
        printf(" Data: ");     
      for(int i = 0; i < MAX_CHARACTER_SIZE; i++){printf("%d ", packet[150+i]);}
        printf("\n");
      

      send(new_socket , packet , sizeof(packet) , 0 ); 
      printf("Message sent\n");      

      counter++;        

      memset(buffer, 0, sizeof(char) * 1024);
  }

  printf("\n\nNumber of Packet sent: %d\n", counter );

  fclose(txFile);
  fclose(dataFile);
}