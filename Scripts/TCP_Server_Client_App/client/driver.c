// Client side C/C++ program to demonstrate Socket programming 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <inttypes.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080 

int64_t S64(const char *s) {
  int64_t i;
  char c ;
  int scanned = sscanf(s, "%" SCNd64 "%c", &i, &c);
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
    
    int sock = 0, valread = 1; 
    struct sockaddr_in serv_addr; 
    char *startTransmit = "Start Transmit";
    char *received = "CLient Acknowledgement"; 
    
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

    char bufferAnchor[1] = {0};     
    char bufferTx[32] = {0};
    //char bufferOffset[21] = {0};
    char bufferData[32] = {0};   

    int packet_content = 0;

    int anchorNumber = 0;
    uint64_t Tx = 0;
    //uint64_t TxOffset = 0;
    int data[32] = {'0'};

    send(sock , startTransmit , strlen(startTransmit) , 0 );

    while(valread == 1)
    {        
        if (packet_content == 0)
        {            
            memset (bufferAnchor, 0, 1);

            valread = read( sock , bufferAnchor, 1);  

            anchorNumber = bufferAnchor[0];
            printf("Anchor Number: %d\n", anchorNumber);

            packet_content++;

        }
        else if (packet_content == 1)
        {
            memset (bufferTx, 0, 32);

            valread = read( sock , bufferTx, 32);

            printf("STx: %s\n", bufferTx);            
            Tx = S64(bufferTx);
            printf("Tx: %lu\n", Tx);

            packet_content++;

            packet_content++;
        }
        else if (packet_content == 2)
        {
            //memset(bufferOffset, 0, 21);
            //valread = read( sock , bufferOffset, 21);
            //TxOffset = S64(bufferOffset);
            //printf("Tx Offset: %lu\n", TxOffset);
            packet_content++;
        }
        else if (packet_content == 3)
        {
            memset (bufferData, 0, 32);

            valread = read( sock , bufferData, 32); 

            printf("Packet Content: \n");           
            for (int i = 0; i < 32; i++)
            {
                data[i] = bufferData[i];
                printf("%d ", data[i] );
            }
            printf("\n"); 

            packet_content++;
        }

        if (packet_content % 4 == 0)
        {
            packet_content = 0;
        }

        send(sock , received , strlen(received) , 0 );
       
    }
          
    

    return 0; 
    
} 
