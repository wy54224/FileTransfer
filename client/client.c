#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include "haffmantree.h"

#define SERVER_PORT 6666
#define SERVER_IP "127.0.0.1"
#define BUFFER_SIZE 1024
typedef int bool;
#define true 1
#define false 0

int main(int argc, char **args){
    int client, data_len;
    struct sockaddr_in client_addr;
    unsigned char buffer[BUFFER_SIZE], tmpbuffer[BUFFER_SIZE + 4];

    if((client = socket(AF_INET,SOCK_STREAM,0)) < 0){
        perror("socket");
        return 0;
    }
    client_addr.sin_family = AF_INET;
    client_addr.sin_port = htons(SERVER_PORT);
    client_addr.sin_addr.s_addr = inet_addr(SERVER_IP);
    if(connect(client, (struct sockaddr *)&client_addr, sizeof(client_addr)) < 0){
        perror("connect");
        return 0;
    }
    send(client, args[1], strlen(args[1]), 0);
    data_len = strlen(args[1]);
    strcpy(tmpbuffer, args[1]);
    tmpbuffer[data_len] = '.';
    tmpbuffer[data_len + 1] = 't';
    tmpbuffer[data_len + 2] = 'm';
    tmpbuffer[data_len + 3] = 'p';
    tmpbuffer[data_len + 4] = '\0';
    memset(buffer, 0, sizeof(buffer));
    FILE *fout;
    if((fout = fopen(tmpbuffer, "wb")) == NULL){
        printf("文件名错误\n");
        return ;
    }
    while(true){
        data_len = recv(client, buffer, BUFFER_SIZE, 0);
        if(data_len < 0){
            perror("recv");
            continue;
        }
        printf("%d\n", data_len);
        fwrite(buffer, sizeof(unsigned char), data_len, fout);
        if(!data_len || data_len < BUFFER_SIZE)
            break;
    }
    fwrite(buffer, sizeof(unsigned char), data_len, fout);
    fclose(fout);
    close(client);
    decode(tmpbuffer, args[1]);
    destroy_all();
    remove(tmpbuffer);
    return 0;
}