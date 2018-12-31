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
void printEncryptedFile(char* ciphertext, 
                        int len, 
                        FILE * encyptFile);

int main(int argc, char const *argv[])
{
	  //check to see if all the argv is entred
    /*if (argc != 5)
    {
      printf("Usage: ./server <inputAudiofile> <password> <outputAudiofile> <encryptfile>\n");
      return 0;
    }   */

    //open the respective files // 44.1 * 15100
    FILE * inputAudiofile;   
    inputAudiofile  = popen("ffmpeg -i input.wav -hide_banner -f s16le -ac 1 -", "r");    

    FILE * tempAudiosampleFile;
    tempAudiosampleFile = fopen("tempAud.txt","w");
     
    // Read, modify and write one sample at a time
    int16_t audioSample;        

    while( fread(&audioSample, 2, 1, inputAudiofile)) // read one 2-byte sample
    {   
        fprintf(tempAudiosampleFile, "%d\n", audioSample);       
    }
 
    pclose(inputAudiofile); 
    fclose(tempAudiosampleFile);

    //opening a temp file to put the values in
    FILE * outputAudiofile;
    outputAudiofile = popen("ffmpeg -y -f s16le -ar 44100 -ac 1 -i - output.wav -hide_banner", "w");

    FILE * tempAudiosampleFileInput;
    tempAudiosampleFileInput = fopen("tempAud.txt","r");

    int16_t audioSampleRead;

    while(fscanf(tempAudiosampleFileInput, "%hd" , &audioSampleRead) != EOF)
    {
        fwrite(&audioSampleRead, 2, 1, outputAudiofile);
    }
     
    // Close input and output pipes       
    pclose(outputAudiofile);    
    fclose(tempAudiosampleFileInput);

    /*FILE *csvfile;
    csvfile = fopen("samInputAudio.csv", "w");
    for (int n = 0; n < N; n++)
    {
    	fprintf(csvfile, "%d\n", bufAud[n] );
    }
    fclose(csvfile);*/

    

    /*
    FILE * outputFile;
    outputFile = fopen(argv[3],"w");

    FILE * encyptFileOutput;
    encyptFileOutput = fopen(argv[4],"w");

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
    printEncryptedFile(IVEncr , mcrypt_enc_get_iv_size(td) , encyptFileOutput);

    //check to see if the key is MAX_CHARACTER_SIZE charcter long
    char * keyEncr = calloc(1, MAX_CHARACTER_SIZE); //MAX_CHARACTER_SIZE * 8 = 128    
    strncpy(keyEncr, argv[2], MAX_CHARACTER_SIZE);	  
	  int keyEncrsize = MAX_CHARACTER_SIZE; /* 256 bits 

	  int bufferEncr_len = MAX_CHARACTER_SIZE;    
    char * bufferEncr = calloc(1, bufferEncr_len); 
	  
    //Encryption algorithm
    printf("==Location-Dependent Algorithm==\n");
    printf("The Key is: %s\n", keyEncr);
    printf("The IV is: %s\n", IVEncr);

    while(fgets(bufferEncr, sizeof bufferEncr, inputFile) != NULL)
    {
      //process buffer           
      encrypt(bufferEncr, bufferEncr_len, IVEncr, keyEncr, keyEncrsize);            
      printEncryptedFile(bufferEncr , bufferEncr_len , encyptFileOutput);     
      //printf("Encryption Part Done\n");     
    }
    if (feof(inputFile)) 
    {
      // hit end of file
      printf("Encryption Process Completed\n");
    }    

    //closing file and free memory after encryption
    fclose(inputFile);
    fclose(encyptFileOutput);

    free(IVEncr);
    free(bufferEncr);
    free(keyEncr);

    //decryption algorithm
    FILE * encyptFileInput;
    encyptFileInput = fopen(argv[4],"r");    

    //initialize decrypt buffer
    int bufferDecr_len = MAX_CHARACTER_SIZE;
    char * bufferDecr = calloc(1, bufferDecr_len);

    //initialize the key
    char * keyDecr = calloc(1, MAX_CHARACTER_SIZE); //MAX_CHARACTER_SIZE * 8 = 128    
    strncpy(keyDecr, argv[2], MAX_CHARACTER_SIZE);    
    int keyDecrsize = MAX_CHARACTER_SIZE;
    
    //getting IV
    char * IVDecr = calloc(1, MAX_CHARACTER_SIZE);
    int IVBuf = 0;

    for (int i = 0; i < MAX_CHARACTER_SIZE; i++)
    {
      fscanf(encyptFileInput, "%d" , &IVBuf);
      IVDecr[i] = IVBuf;
    }

    printf("The IV is: %s\n", IVDecr);
    printf("The Key is: %s\n", keyDecr);

    //initilizing he tempAESbuffer and bufferIndex
    int  AESbuf = 0;
    int bufIndex = 0;

    //getting the AES blocks and decrypting it
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
        //printf("Decryption Part Done\n");      

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
      //printf("Decryption Part Done\n");  
    } 

    printf("Decryption Process Completed\n");

    //closing the output file and free memory    
    fclose(encyptFileInput);
    fclose(outputFile);

    free(keyDecr);
    free(bufferDecr);    
    free(IVDecr);

    return 0;*/

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
