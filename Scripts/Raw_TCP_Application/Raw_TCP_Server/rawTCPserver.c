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

int main (void)
{
  	int s = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);  /* open raw socket */

	struct sockaddr_in sin;
  	sin.sin_family = AF_INET;
  	sin.sin_port = htons (P);
  	sin.sin_addr.s_addr = inet_addr ("127.0.0.1"); 
  
  	char datagram[4096];   
  	memset (datagram, 0, 4096);

  	char * data;
  	char * code;

  	code = "0xABCD";
  	data = "Source IP: 192.168.1.12 Port: 2016  Destination IP: 192.168.1.18 Port: 2020 This is a test message.";
  
 	unsigned short int ip_len; 
	char * newDatagram = createDatagram(datagram, P, code, data, sin, &ip_len);


/* finally, it is very advisable to do a IP_HDRINCL call, to make sure
   that the kernel knows the header is included in the data, and doesn't
   insert its own header into the packet before our data */
         
    int one = 1;
    const int *val = &one;
    if (setsockopt (s, IPPROTO_IP, IP_HDRINCL, val, sizeof (one)) < 0)
    {    	
     	printf ("Warning: Cannot set HDRINCL!\n");
    }
    

  	while (1)
    {
      if (sendto (s, newDatagram, ip_len, 0, (struct sockaddr *) &sin, sizeof (sin)) < 0)    
    	{
    		printf ("error\n");
    	}  
      else
      	{
      		printf ("send\n");
      		break;      		
     	}  
  
    }

  return 0;
}
