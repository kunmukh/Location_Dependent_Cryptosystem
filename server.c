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
    }   

    //open the respective files
    FILE * inputFile;
    inputFile = fopen(argv[1],"r");

    //check to see if the key is 32 charcter long
    char * key; //32 * 8 = 128
    key = calloc(1, 32);
    strncpy(key, argv[2], 32);

    FILE * outputFile;
    outputFile = fopen(argv[3],"w");

    FILE * encyptFile;
    encyptFile = fopen(argv[4],"w");

    //create a MCRYPT to get certain info
    MCRYPT td = mcrypt_module_open("rijndael-256", NULL, "cbc", NULL);
	  
	  //A random block should be placed as the first block (IV) 
	  //so the same block or messages always encrypt to something different.
	  char * IV = malloc(mcrypt_enc_get_iv_size(td)); //return 8
	  FILE *fp;
    fp = fopen("/dev/urandom", "r");
    fread(IV, 1, mcrypt_enc_get_iv_size(td), fp);
    fclose(fp);
    mcrypt_generic_end(td);	  
	  
	  int keysize = 32; /* 256 bits */ 
	  int buffer_len = 32;
    char * buffer;

	  buffer = calloc(1, buffer_len);    

    while(fgets(buffer, sizeof buffer, inputFile) != NULL)
    {
      //process buffer     
      printf("==Location-Dependent Algorithm==\n");      
      encrypt(buffer, buffer_len, IV, key, keysize);
      printf("Encryption Completed\n"); 

      printf("cipher:  "); 
      printEncryptedFile(buffer , buffer_len , encyptFile);
      
      decrypt(buffer, buffer_len, IV, key, keysize);
      fprintf(outputFile, "%s", buffer);
      printf("Decryption Completed\n");      
    }
    if (feof(inputFile)) 
    {
      // hit end of file
      printf("Process Completed\n");
    }  

    fclose(inputFile);
    fclose(outputFile);
    fclose(encyptFile);
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
  printf("\n");
  fprintf(encyptFile, "%s", "\n");
}
