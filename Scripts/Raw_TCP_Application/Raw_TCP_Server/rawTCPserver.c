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
#include "tcp_ip_packet.h"

#define P 25    /* lets flood the sendmail port */ 
#define MAX_CHARACTER_SIZE 32

int main (int argc, char const *argv[])
{
  	int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);  /* open raw socket */

	struct sockaddr_in sin;
  	sin.sin_family = AF_INET;
  	sin.sin_port = htons (P);
  	sin.sin_addr.s_addr = inet_addr ("127.0.0.1"); 
  
  	char datagram[4096];   
  	memset (datagram, 0, 4096);

  	char * code;
  	char * data = calloc(0, MAX_CHARACTER_SIZE);  	

  	FILE * textin;
    textin = fopen(argv[1],"r");

  	int AESbuf = 0, bufIndex = 0 ;

  	code = "0xABCD";

    int counter = 0;

  	while(fscanf(textin, "%d" , &AESbuf) != EOF)
    {        
      	if(bufIndex < MAX_CHARACTER_SIZE)
      	{
     	      data[bufIndex] = AESbuf;
     	      bufIndex++;  
     	  }

       	else
       	{             
         	 	unsigned short int ip_len; 
         		char * newDatagram = createDatagram(datagram, P, code, data, sin, &ip_len);
         	         
         	    //***
         	    int one = 1;
         	    const int *val = &one;
         	    if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
         	    {    	
         	     	printf ("Warning: Cannot set HDRINCL!\n");
         	    }  
         	    //****

         	    if (sendto (s, newDatagram, ip_len, 0, (struct sockaddr *) &sin, sizeof (sin)) < 0)    
         	    {
         	    	printf ("error\n");
         	    }  
         	    else
         	    {
         	      	printf ("send Packtet Num: %d \n", counter); 
                  counter++;      	      	       	      	      		
         	    }

              bufIndex = 0;
              memset (datagram, 0, 32);

              data[bufIndex] = AESbuf;
              bufIndex++; 

              usleep(2);

     		}	  
  
    }

    code = "0xABCE";
    unsigned short int ip_len; 
    char * newDatagram = createDatagram(datagram, P, code, data, sin, &ip_len);
         
    //***
    int one = 1;
    const int *val = &one;
    if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    {     
      printf ("Warning: Cannot set HDRINCL!\n");
    }  
    //****
    if (sendto (s, newDatagram, ip_len, 0, (struct sockaddr *) &sin, sizeof (sin)) < 0)    
    {
      printf ("error\n");
    }  
    else
    {
        counter++; 
        printf ("send Terminating Sequence. Packet Num: %d Last Char: %c\n", counter 
          , data[MAX_CHARACTER_SIZE -1]);                 
    }

    free (data);
    

  return 0;
}
