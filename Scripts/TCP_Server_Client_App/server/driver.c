#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <inttypes.h>

#define PORT 8080 

#define MAX_CHARACTER_SIZE 32

int main(int argc, char const *argv[]) 
{ 
    int server_fd, new_socket; 
    struct sockaddr_in address; 
    int opt = 1; 

    int addrlen = sizeof(address);      
       
    // Creating socket file descriptor 
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    { 
        perror("socket failed"); 
        exit(EXIT_FAILURE); 
    } 
       
    // Forcefully attaching socket to the port 8080 
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, 
                                                  &opt, sizeof(opt))) 
    { 
        perror("setsockopt"); 
        exit(EXIT_FAILURE); 
    } 
    address.sin_family = AF_INET; 
    address.sin_addr.s_addr = INADDR_ANY; 
    address.sin_port = htons( PORT ); 
       
    // Forcefully attaching socket to the port 8080 
    if (bind(server_fd, (struct sockaddr *)&address,  
                                 sizeof(address))<0) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 

    if (listen(server_fd, 3) < 0) 
    { 
        perror("listen"); 
        exit(EXIT_FAILURE); 
    } 
    if ((new_socket = accept(server_fd, (struct sockaddr *)&address,  
                       (socklen_t*)&addrlen))<0) 
    { 
        perror("accept"); 
        exit(EXIT_FAILURE); 
    } 
    
    char bufferAnchor[1] = {0};     
    char bufferTx[21] = {0};
    //char bufferOffset[21] = {0};
    char bufferData[32] = {0};
    char buffer[1024] = {0};

    FILE * transmissionFile;
    transmissionFile = fopen(argv[1],"r");

    FILE * dataFile;
    dataFile = fopen(argv[2],"r");
    
    int AESbuf = 0;    
    int anchorNumber = 0;
    uint64_t Tx = 0;
    int packet_content = 0;

    while(read( new_socket , buffer, 1024))
    {
        printf("%s\n",buffer ); 

        if (packet_content == 0)
        {
            memset (bufferAnchor, 0, 1);
            if (fscanf(transmissionFile, "%d" , &anchorNumber) != EOF)
            {
                bufferAnchor[0] = anchorNumber;
            }
            printf("Anchor: %c\n", bufferAnchor[0] );
            send(new_socket , bufferAnchor , strlen(bufferAnchor) , 0 );
            packet_content++;
            printf("Anchor Number Sent\n");
        }
        else if (packet_content == 1)
        {
            memset (bufferTx, 0, 21);
            if (fscanf(transmissionFile, "%lu" , &Tx) != EOF)
            {                
                sprintf(bufferTx, "%" PRIu64, Tx);
            }
            printf("Tx: %s\n", bufferTx);            
            send(new_socket , bufferTx , strlen(bufferTx) , 0 );
            packet_content++;

            packet_content++;
            printf("Tx sent\n");
        }
        else if (packet_content == 2)
        {
            //memset(bufferOffset, 0, 21);
            //send(new_socket , bufferOffset , strlen(bufferOffset) , 0 );
            //packet_content++;
        }
        else if (packet_content == 3)
        {
            memset (bufferData, 0, 32);
            for (int i = 0; i < MAX_CHARACTER_SIZE; i++)
            {
                if (fscanf(dataFile, "%d" , &AESbuf) != EOF)
                {
                     bufferData[i] = AESbuf;
                }                                        
            }
            printf("Data %s\n", bufferData);
            send(new_socket , bufferData , strlen(bufferData) , 0 );
            packet_content++;
            printf("Data sent.\n");
        }                
        memset (buffer, 0, 1024);
        
        if (packet_content % 4 == 0)
        {
            packet_content = 0;
        }
    }  

    fclose(dataFile);
    fclose(transmissionFile);
    return 0; 
} 