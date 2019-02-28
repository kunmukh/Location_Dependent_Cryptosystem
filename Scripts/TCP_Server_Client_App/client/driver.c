// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <unistd.h>
#include <arpa/inet.h>
#include <inttypes.h>

#define PORT 8080 

int64_t S64(const char *s) {
  int64_t i;
  char c ;
  int scanned = sscanf(s, "%"  PRIu64 "%c", &i, &c);
  if (scanned == 1) return i;
  if (scanned > 1) {
    // TBD about extra data found
    return i;
    }
  // TBD failed to scan;  
  return 0;  
}

int main(int argc, char const *argv[]) 
{ 
    int sock = 0; 
    struct sockaddr_in serv_addr; 
    char *hello = "Hello from client"; 
    char buffer[1024] = {0}; 
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
    { 
        printf("\n Socket creation error \n"); 
        return -1; 
    } 

    memset(&serv_addr, '0', sizeof(serv_addr)); 

    serv_addr.sin_family = AF_INET; 
    serv_addr.sin_port = htons(PORT); 
    
    // Convert IPv4 and IPv6 addresses from text to binary form 
    if(inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr)<=0) 
    { 
        printf("\nInvalid address/ Address not supported \n"); 
        return -1; 
    } 

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) 
    { 
        printf("\nConnection Failed \n"); 
        return -1; 
    } 

    send(sock , hello , strlen(hello) , 0 );
    printf("Ready to receive\n");

    char * received = "Packet acknowledgement received";

    send(sock , received , strlen(received) , 0 ); 
    int counter = 0;

    uint64_t anchorNumber = 0;
    uint64_t tx = 0;
    uint64_t offset = 0;

    char txBuff[21]= {0};
    char anchorNumberbuff[21]= {0};
    char offsetBuff[21]= {0};   

    int index = 0;

    FILE * receptionFile;
    receptionFile = fopen(argv[1],"w");

    while (read( sock , buffer, 1024) > 1)
    {
        printf("\nPacket Content: %s\n", buffer);
        
        for(int i =0; i <1; i++)
            {anchorNumberbuff[index] = buffer[i]; index++;}
        index = 0;        
        for(int i =1; i <21; i++)
            {txBuff[index] = buffer[i]; index++;}
        index = 0;        
        for (int i = 21; i < strlen(buffer); i++)
            {offsetBuff[index] = buffer[i]; index++;}
        printf("\n");index = 0;        

        anchorNumber =  S64(anchorNumberbuff);
        tx = S64(txBuff);
        offset = S64(offsetBuff);        

        printf("Anchor Number: %lu\n", anchorNumber);
        printf("Tx: %lu\n", tx);
        printf("Offset %lu\n", offset);

        fprintf(receptionFile, "%lu ", anchorNumber);
        fprintf(receptionFile, "%lu\n",tx);

        send(sock , received , strlen(received) , 0 ); 

        counter++;

        memset(buffer, 0, sizeof(char) * 1024);
        memset(txBuff, 0, sizeof(char) * 21);
        memset(anchorNumberbuff, 0, sizeof(char) * 21);
        memset(offsetBuff, 0, sizeof(char) * 21);
    } 
    
    printf("Number of Packet sent: %d\n", counter );

    return 0; 
} 
