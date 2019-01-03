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

int main(int argc, char const *argv[])
{
	int fd = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);

	if (fd == -1)
	{
		printf("Socket cannot be opend. Need root permission\n");
		return -1;
	}
	char buffer[8192]; /* single packets are usually not bigger than 8192 bytes */

	while (read (fd, buffer, 8192) > 0)
	{
		printf ("Caught tcp packet: %s\n", 
			buffer+sizeof(struct iphdr)+sizeof(struct tcphdr));

		memset (buffer, 0, 8192);
	}

	return 0;
}