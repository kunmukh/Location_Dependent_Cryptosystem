// File: Makefile
// Class: EE 495-Senior Reseach                      # Instructor: Dr. Don Roberts
// Assignment: Location-Dept Cryptosystem            # Date started: 12/26/2018
//Programmer: Kunal Mukherjee                       # Date completed:

//adding the header files
#include <stdio.h>
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <mcrypt.h> //http://linux.die.net/man/3/mcrypt

//defining the max charc size
#define MAX_CHARACTER_SIZE 32

//the encrypt function
int encrypt(
    void* buffer,
    int buffer_len, /* Because the plaintext could include null bytes*/
    char* IV, //initilization vector
    char* key,
    int key_len 
);

//the decrypt function
int decrypt(
    void* buffer,
    int buffer_len,
    char* IV, 
    char* key,
    int key_len 
);

//cipher text displyer
void printEncryptedFile(char* ciphertext, int len, FILE * encyptFile);


int main(int argc, char const *argv[])
{
	  //check to see if all the argv is entred
    if (argc != 5)
    {
      printf("Usage: ./server <inputfile> <password> <outputfile> <encryptfile>\n");
      return 0;
    }   

    //open the respective files
    FILE * inputFile;
    inputFile = fopen(argv[1],"r");    

    FILE * outputFile;
    outputFile = fopen(argv[3],"w");

    FILE * encyptFileOutput;
    encyptFileOutput = fopen(argv[4],"w");

    //create a MCRYPT to get certain info
    MCRYPT td = mcrypt_module_open("rijndael-256", NULL, "cbc", NULL);
	  
	  //A random block should be placed as the first block (IV) 
	  //so the same block or messages always encrypt to something different.
	  char * IV = malloc(mcrypt_enc_get_iv_size(td)); //return 8
	  FILE * fp;
    fp = fopen("/dev/urandom", "r");
    fread(IV, 1, mcrypt_enc_get_iv_size(td), fp);
    fclose(fp);
    mcrypt_generic_end(td);	  

    //check to see if the key is MAX_CHARACTER_SIZE charcter long
    char * key; //MAX_CHARACTER_SIZE * 8 = 128
    key = calloc(1, MAX_CHARACTER_SIZE);
    strncpy(key, argv[2], MAX_CHARACTER_SIZE);
	  
	  int keysize = MAX_CHARACTER_SIZE; /* 256 bits */ 
	  int buffer_len = MAX_CHARACTER_SIZE;
    
    char * buffer;
	  buffer = calloc(1, buffer_len);    

    //Encryption algorithm
    printf("==Location-Dependent Algorithm==\n");
    printf("The Key is: %s\n", key);
    printf("The IV is: %s\n", IV);

    while(fgets(buffer, sizeof buffer, inputFile) != NULL)
    {
      //process buffer           
      encrypt(buffer, buffer_len, IV, key, keysize);            
      printEncryptedFile(buffer , buffer_len , encyptFileOutput);     
      //printf("Encryption Part Done\n");     
    }
    if (feof(inputFile)) 
    {
      // hit end of file
      printf("Encryption Process Completed\n");
    }    
    fclose(inputFile);
    fclose(encyptFileOutput); 
    free(buffer); 

    //decryption algorithm
    FILE * encyptFileInput;
    encyptFileInput = fopen(argv[4],"r");
    
    //clear buffer
    buffer = calloc(1, buffer_len);

    //decryption part started
    int  AESbuf;
    int bufIndex = 0;

    while(fscanf(encyptFileInput, "%d" , &AESbuf) != EOF)
    {        
      if(bufIndex < MAX_CHARACTER_SIZE)
      {
        buffer[bufIndex] = AESbuf;
        bufIndex++;  
      }
      else
      {             
        decrypt(buffer, buffer_len, IV, key, keysize);
        fprintf(outputFile, "%s", buffer);
        //printf("Decryption Part Done\n");      

        bufIndex = 0;
        memset(buffer, 0 , MAX_CHARACTER_SIZE);
        buffer[bufIndex] = AESbuf;
        bufIndex++;
      }        
    }

    if( strlen(buffer) != 0)
    {
      decrypt(buffer, buffer_len, IV, key, keysize);
      fprintf(outputFile, "%s", buffer);
      //printf("Decryption Part Done\n");  
    } 

    printf("Decryption Process Completed\n");

    fclose(encyptFileInput);
    fclose(outputFile);

    free(key);
    free(buffer);
    return 0;
}	

int encrypt(void* buffer, int buffer_len, char* IV, char* key, int key_len)
{
  MCRYPT td = mcrypt_module_open("rijndael-256", NULL, "cbc", NULL);
  int blocksize = mcrypt_enc_get_block_size(td);
  if( buffer_len % blocksize != 0 ){return 1;}

  mcrypt_generic_init(td, key, key_len, IV);
  mcrypt_generic(td, buffer, buffer_len);
  mcrypt_generic_deinit (td);
  mcrypt_module_close(td);
  
  return 0;
}

int decrypt(void* buffer, int buffer_len, char* IV, char* key, int key_len)
{
  MCRYPT td = mcrypt_module_open("rijndael-256", NULL, "cbc", NULL);
  int blocksize = mcrypt_enc_get_block_size(td);
  if( buffer_len % blocksize != 0 ){return 1;}
  
  mcrypt_generic_init(td, key, key_len, IV);
  mdecrypt_generic(td, buffer, buffer_len);
  mcrypt_generic_deinit (td);
  mcrypt_module_close(td);
  
  return 0;
}

//displays as well as writes the encrypt file
void printEncryptedFile(char* ciphertext, int len, FILE * encyptFile)
{
  int v;
  for (v=0; v<len; v++)
  {
    //printf("%d ", ciphertext[v]);
    fprintf(encyptFile, "%d", ciphertext[v]);
    fprintf(encyptFile, "%s", " ");
  }  
  
  fprintf(encyptFile, "%s", "\n");
}
