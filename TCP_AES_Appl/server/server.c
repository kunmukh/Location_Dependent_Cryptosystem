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
#define MAX_TIME_ANCHOR 32
#define C 300000000
#define PACKET_LENGTH 900

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
void txValueFromKey(char const * key, char const * d1, 
                    char const * d2, char const * d3);

//sends the TCP value
void TCPServiceRoutine();

//Creates Password from AES
void passwordCreation(char const * key, char * pass);


int main(int argc, char const *argv[])
{
	  //check to see if all the argv is entred
    if (argc != 5)
    {
      printf("Usage: ./server <password> <d1> <d2> <d3>\n");
      return 0;
    }  

    audioToAESConversion(argv[1]);

    txValueFromKey(argv[1], argv[2], argv[3], argv[4]);

    TCPServiceRoutine();
    
    return 0;
}	

//the function that creates the AUDIO->AES
void audioToAESConversion(char const * key)
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

    //Generate the password
    unsigned char *obuf = SHA256(key, strlen(key), 0);
    char * pass = calloc(1, SHA256_DIGEST_LENGTH);
    
    printf("AES-258: %s : ", key);   
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
      pass[i] = obuf[i];
      printf("%02x", obuf[i]);               
    } 
    printf("\n"); 

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
    printf("\nTx Value Creation starts \n");

    //input buffer
    char * ibuf = calloc(1, MAX_CHARACTER_SIZE);  
    strncpy(ibuf, key, MAX_CHARACTER_SIZE);

    printf("\nPassword: %s\n", ibuf);
     //output buffer
    unsigned char *obuf = SHA256(ibuf, strlen(ibuf), 0);
    
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
        printf("%02x", obuf[i]);
    }
    printf("\n\n");

    //create the Tx array
    int oTx [MAX_CHARACTER_SIZE] = {'0'};
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) 
    {
        oTx[i] = obuf[i];
        //printf("%d\n", oTx[i]);
    }
    
    int dA = atoi(d1);    
    int dB = atoi(d2);    
    int dC = atoi(d3);

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
    fprintf(transmissionFile, "%lu\n", Txfirst);

    FILE * dataFile;
    dataFile = fopen("EncrFileOutput.txt","r");
    int i = 0, breakflag = 0, data = 0, numPak = 0;

    FILE * debugFile;
    debugFile = fopen("Debug.txt","w");    

    int bufferNumOld = 0, bufferNum = 0;

    while(1)    
    {         
      bufferNumOld = bufferNum;

      for (int j = 0; j < PACKET_LENGTH; j++)
      {
        if (fscanf(dataFile, "%d", &data) != EOF)
        {
          bufferNum++;
        }
        else
        {
          breakflag = 1;
          break;
        }
      }      

      if (i < 32)
      {
        Slot = oTx[i];
        
        if (rand() % 2 == 0)
        {       
          Tx = Ttxlast + Tdistlast + TbtwnOffset - TdistA + ((Slot + 0.5) * Tslot); 
          Tdistlast = TdistA;
          printf("AES: %02x Slot: %d Anchor A: %lu \n", oTx[i], oTx[i], Tx);
          fprintf(transmissionFile, "%d ", 0); 

          fprintf(debugFile, "%d ", 0);
          fprintf(debugFile, "%d ", Slot);
          fprintf(debugFile, "%d ", (int)(TdistA + ((Slot + 0.5) * Tslot)));
        }
        else if (rand() % 3 == 0)
        {
          Tx = Ttxlast + Tdistlast + TbtwnOffset - TdistB + ((Slot + 0.5) * Tslot);
          Tdistlast = TdistB; 
          printf("AES: %02x Slot: %d Anchor B: %lu \n", oTx[i], oTx[i], Tx);
          fprintf(transmissionFile, "%d ", 1); 

          fprintf(debugFile, "%d ", 1);
          fprintf(debugFile, "%d ", Slot);
          fprintf(debugFile, "%d ", (int)(TdistB + ((Slot + 0.5) * Tslot)));
        }
        else 
        {
          Tx = Ttxlast + Tdistlast + TbtwnOffset - TdistC + ((Slot + 0.5) * Tslot); 
          Tdistlast = TdistC;
          printf("AES: %02x Slot: %d Anchor C: %lu \n", oTx[i], oTx[i], Tx);
          fprintf(transmissionFile, "%d ", 2); 

          fprintf(debugFile, "%d ", 2); 
          fprintf(debugFile, "%d ", Slot);
          fprintf(debugFile, "%d ", (int)(TdistC + ((Slot + 0.5) * Tslot)));         
        }
              
        fprintf(transmissionFile, "%lu\n",Tx);

        fprintf(debugFile, " %d %d\n",bufferNumOld, bufferNum);        

        Ttxlast = Tx;
        i++;
        numPak++;

        if(i == 32)
        {
          i = 0;
        }
      }      

      if(breakflag)
      {
        break;
      }
           
    }


    printf("Num Pak:%d\n",numPak);
    fclose(transmissionFile);
    fclose(dataFile);
    fclose(debugFile);
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

  char packet [1024] = {0};
  memset(packet, 0, sizeof(char) * 1024);
  FILE * txFile;
  txFile = fopen("transmission.dat","r");
  FILE * dataFile;
  dataFile = fopen("EncrFileOutput.txt","r");

  uint64_t anchorNumber = 0;
  uint64_t Tx = 0, Txstart = 0;  
  int data = 0;

  char txBuff[21]= {0};
  char anchorNumberbuff[21]= {0};
  
  char dataBuff[PACKET_LENGTH]={0};
  int breakflag = 0;

  fscanf(txFile, "%lu" , &Txstart);

  while (read( new_socket , buffer, 1024) > 1)
  {        
      memset(packet, 0, sizeof(char) * 1000);

      printf("%s\n",buffer);

      for (int i = 0; i < PACKET_LENGTH; i++)
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

      fscanf(txFile, "%lu" , &anchorNumber);      
      sprintf(anchorNumberbuff, "%" PRIu64, anchorNumber);

      fscanf(txFile, "%lu" , &Tx);
      sprintf(txBuff, "%" PRIu64, Tx);        

      memcpy(&packet[0], anchorNumberbuff, sizeof(anchorNumberbuff));
      memcpy(&packet[50], txBuff, sizeof(txBuff));      
      memcpy(&packet[100], dataBuff, sizeof(dataBuff));     

      printf("\n\nPacket Content: AcNum:%s Tx:%s Counter:%d\n", &packet[0], 
        &packet[50], counter);
      /*printf(" Data: ");     
      for(int i = 0; i < PACKET_LENGTH; i++){printf("%d ", packet[100+i]);}
      printf("\n");*/

      send(new_socket , packet , sizeof(packet) , 0 ); 
      printf("Message sent\n");      

      counter++;        

      memset(buffer, 0, sizeof(char) * 1024);
      memset(packet, 0, sizeof(char) * 1024);
      memset(anchorNumberbuff, 0, sizeof(char) * 21);
      memset(txBuff, 0, sizeof(char) * 21);
      memset(dataBuff, 0, sizeof(char) * PACKET_LENGTH);

      if (breakflag == 1)
      {
        break;
      }
  }

  printf("\n\nNumber of Packet sent: %d\n", counter );

  fclose(txFile);
  fclose(dataFile);
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