struct ipheader {
 unsigned char ip_hl:4, ip_v:4; /* this means that each member is 4 bits */
 unsigned char ip_tos;
 unsigned short int ip_len;
 unsigned short int ip_id;
 unsigned short int ip_off;
 unsigned char ip_ttl;
 unsigned char ip_p;
 unsigned short int ip_sum;
 unsigned int ip_src;
 unsigned int ip_dst;
}; /* total ip header length: 20 bytes (=160 bits) */

 struct tcpheader {
 unsigned short int th_sport;
 unsigned short int th_dport;
 unsigned int th_seq;
 unsigned int th_ack;
 unsigned char th_x2:4, th_off:4;
 unsigned char th_flags;
 unsigned short int th_win;
 unsigned short int th_sum;
 unsigned short int th_urp;
}; /* total tcp header length: 20 bytes (=160 bits) */

unsigned short   /* this function generates header checksums */
csum (unsigned short *buf, int nwords)
{
  unsigned long sum;
  for (sum = 0; nwords > 0; nwords--)
    sum += *buf++;
  sum = (sum >> 16) + (sum & 0xffff);
  sum += (sum >> 16);
  return ~sum;
}

char * createDatagram(char * datagram, int P, char * code, char * data, struct sockaddr_in sin, 
	unsigned short int * len)
{
 	
	char * newDatagram = datagram;

 	struct ip *iph = (struct ip *) datagram;
	struct tcphdr *tcph = (struct tcphdr *) datagram + sizeof (struct ip);

	strcpy(&datagram[sizeof (struct ip) + sizeof (struct tcphdr)], code);
	strcpy(&datagram[sizeof (struct ip) + sizeof (struct tcphdr)] + strlen (code), data); 

	/* we'll now fill in the ip/tcp header values, see above for explanations */
  	iph->ip_hl = 5;
  	iph->ip_v = 4;
  	iph->ip_tos = 0;
  	iph->ip_len = sizeof (struct ip) + sizeof (struct tcphdr) + strlen (code)+ strlen (data); 
  	iph->ip_id = htonl (54321); /* the value doesn't matter here */
  	iph->ip_off = 0;
  	iph->ip_ttl = 255;
  	iph->ip_p = 6;
  	iph->ip_sum = 0;    /* set it to 0 before computing the actual checksum later */
  	iph->ip_src.s_addr = inet_addr ("1.2.3.4");/* SYN's can be blindly spoofed */
  	iph->ip_dst.s_addr = sin.sin_addr.s_addr;

  	tcph->th_sport = htons (1234);  /* arbitrary port */
  	tcph->th_dport = htons (P);
  	tcph->th_seq = random ();/* in a SYN packet, the sequence is a random */
  	tcph->th_ack = 0;/* number, and the ack sequence is 0 in the 1st packet */
  	tcph->th_x2 = 0;
  	tcph->th_off = 0;   /* first and only tcp segment */
  	tcph->th_flags = TH_SYN;  /* initial connection request */
  	tcph->th_win = htonl (65535); /* maximum allowed window size */
  	tcph->th_sum = 0;/* if you set a checksum to zero, your kernel's IP stack
                    should fill in the correct checksum during transmission */
  	tcph->th_urp = 0;

  	iph->ip_sum = csum ((unsigned short *) datagram, iph->ip_len >> 1);

  	*len = iph->ip_len;

  	return newDatagram;
}