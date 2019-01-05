#define __USE_BSD /* use bsd'ish ip header */
#include <sys/socket.h> /* these headers are for a Linux system, but */
#include <netinet/in.h> /* the names on other systems are easy to guess.. */
#include <netinet/ip.h>
#define __FAVOR_BSD /* use bsd'ish tcp header */
#include <netinet/tcp.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>


#define MAX_CHARACTER_SIZE 32

//displays as well as writes the encrypt file
void printEncryptedFile(char* ciphertext, int len, FILE* encyptFile)
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

int main(int argc, char const *argv[])
{
	int fd = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);

	if (fd == -1)
	{
		printf("Socket cannot be opend. Need root permission\n");
		return -1;
	}

	FILE * textOut;
    textOut = fopen("textOut.txt","w");

	char buffer[8192]; /* single packets are usually not bigger than 8192 bytes */	

	int counter = 0;

	while (read (fd, buffer, 8192) > 0)
	{		
		if (strncmp( &buffer[ sizeof(struct iphdr) + sizeof(struct tcphdr)],
		 "0xABCD", strlen("0xABCD")) == 0)
		{
			printf("TCP Pack Received. Packet Number: %d\n", counter);
			printEncryptedFile(
				&buffer[ sizeof(struct iphdr) + sizeof(struct tcphdr) + strlen("0xABCD")],
				 MAX_CHARACTER_SIZE, textOut);
			counter++;			
		}

		if (strncmp( &buffer[ sizeof(struct iphdr) + sizeof(struct tcphdr)],
		 "0xABCE", strlen("0xABCE")) == 0)
		{
			printf("TCP Pack Received.\n");
			printEncryptedFile(
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

	return 0;
}