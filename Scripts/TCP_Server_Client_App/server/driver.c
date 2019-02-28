// Server side C/C++ program to demonstrate Socket programming 
#include <unistd.h> 
#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <string.h> 
#include <inttypes.h>
#define PORT 8080 
int main(int argc, char const *argv[]) 
{ 
    int server_fd, new_socket, valread; 
    struct sockaddr_in address; 
    int opt = 1; 
    int addrlen = sizeof(address); 
    char buffer[1024] = {0}; 
    
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

    valread = read( new_socket , buffer, 1024);     
    printf("%s\n",buffer ); 
    char * message = "I know you";

    int counter = 0;

    char packet [100] = {0};
    memset(packet, 0, sizeof(char) * 100);
    FILE * transmissionFile;
    transmissionFile = fopen(argv[1],"r");

    uint64_t anchorNumber = 0;
    uint64_t Tx = 0;
    uint64_t offset = 599;

    char txBuff[21]= {0};
    char anchorNumberbuff[21]= {0};
    char offsetBuff[21]= {0};

    while (read( new_socket , buffer, 1024) > 1)
    {
        printf("%s\n",buffer );

        if (fscanf(transmissionFile, "%lu" , &anchorNumber) == EOF){break;}
        sprintf(anchorNumberbuff, "%" PRIu64, anchorNumber);
        if (fscanf(transmissionFile, "%lu" , &Tx) == EOF){break;}
        sprintf(txBuff, "%" PRIu64, Tx);
        sprintf(offsetBuff, "%" PRIu64, offset);        

        memcpy(&packet[0], anchorNumberbuff, sizeof(anchorNumberbuff));
        memcpy(&packet[strlen(anchorNumberbuff)], txBuff, sizeof(txBuff));
        memcpy(&packet[strlen(anchorNumberbuff) + strlen(txBuff)], offsetBuff, 
            sizeof(offsetBuff));

        printf("%s\n", packet);

        send(new_socket , packet , strlen(packet) , 0 ); 
        printf("Message sent\n\n");       
        memset(packet, 0, sizeof(char) * 100);

        counter++;
        if (counter > 100) {break;}
    }

    printf("%d\n", counter );
    

    return 0; 
} 
