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
void display(char* ciphertext, int len);


int main(int argc, char const *argv[])
{
	  MCRYPT td = mcrypt_module_open("rijndael-256", NULL, "cbc", NULL);
	  char * plaintext = "test text 123test text 123";
	  //A random block should be placed as the first block (IV) 
	  //so the same block or messages always encrypt to something different.
	  char * IV = malloc(mcrypt_enc_get_iv_size(td)); //return 8
	  FILE *fp;
    fp = fopen("/dev/urandom", "r");
    fread(IV, 1, mcrypt_enc_get_iv_size(td), fp);
    fclose(fp);
    mcrypt_generic_end(td);

	  char * key = "0123456789abcdef0123456789abcdef"; //32 * 8 = 128
	  char * buffer;
	  int keysize = 32; /* 256 bits */ 
	  int buffer_len = 32;

	  buffer = calloc(1, buffer_len);
	  strncpy(buffer, plaintext, buffer_len);

	  printf("==C==\n");
	  printf("plain:   %s\n", plaintext);
	  encrypt(buffer, buffer_len, IV, key, keysize); 
	  printf("cipher:  "); 
	  display(buffer , buffer_len);
	  decrypt(buffer, buffer_len, IV, key, keysize);
	  printf("decrypt: %s\n", buffer);

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

void display(char* ciphertext, int len)
{
  int v;
  for (v=0; v<len; v++)
  {
    printf("%d ", ciphertext[v]);
  }
  printf("\n");
}
