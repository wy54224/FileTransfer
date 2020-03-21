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
#define EPOLL_SIZE 500
#define LISTEN_SIZE 20
#define BUFFER_SIZE 1024
typedef int bool;
#define true 1
#define false 0

int setnonblocking(int sockfd){
    fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)| O_NONBLOCK);
    return 0;
}

void addfd(int epollfd, int fd, bool enable_et){
    struct epoll_event ev;
    ev.data.fd = fd;
    ev.events = EPOLLIN;
    if(enable_et)
        ev.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
    setnonblocking(fd);
    printf("fd added to epoll!\n\n");
}

int main(){
    int server, client, data_len, epfd, i;
    socklen_t addr_len;
    struct sockaddr_in server_addr, client_addr;
    unsigned char buffer[BUFFER_SIZE], tmpbuffer[BUFFER_SIZE + 4], c;
    struct epoll_event events[EPOLL_SIZE];

    if((server = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("socket");
        return 0;
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // 端口绑定
    if(bind(server, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0){
        perror("bind");
        return 0;
    }
    // 监听
    if(listen(server, LISTEN_SIZE) < 0){
        perror("listen");
        return 0;
    }

    // epoll创建事件表
    if((epfd = epoll_create(EPOLL_SIZE)) < 0){
        perror("epoll_create");
        return 0;
    }
    addfd(epfd, server, true);

    while(1){
        int epoll_events_count = epoll_wait(epfd, events, EPOLL_SIZE, -1);
        if(epoll_events_count < 0){
            perror("epoll_wait");
            break;
        }
        printf("epoll_events_count = %d\n", epoll_events_count);
        for(i = 0; i < epoll_events_count; ++i){
            int sockfd = events[i].data.fd;
            if(sockfd == server){
                addr_len = sizeof(struct sockaddr_in);
                if((client = accept(server, (struct sockaddr*)&client_addr, &addr_len)) < 0){
                    perror("accept");
                    continue;
                }
                printf("IP %s, port %u\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

                addfd(epfd, client, true);
            }else{
                data_len = recv(client, buffer, BUFFER_SIZE, 0);
                if(data_len < 0){
                    perror("recv");
                    continue;
                }
                buffer[data_len] = '\0';
                if(!data_len){
                    close(sockfd);
                    continue;
                }

                strcpy(tmpbuffer, buffer);
                tmpbuffer[data_len] = '.';
                tmpbuffer[data_len + 1] = 't';
                tmpbuffer[data_len + 2] = 'm';
                tmpbuffer[data_len + 3] = 'p';
                tmpbuffer[data_len + 4] = '\0';
                printf("%s %s\n", buffer, tmpbuffer);

                buildHaffmanTreeFromFile(buffer);
                encode(buffer, tmpbuffer);
                destroy_all();
                
                memset(buffer, 0, sizeof(buffer));
                FILE *fin;
                if((fin = fopen(tmpbuffer, "rb")) == NULL){
                    printf("文件名错误\n");
                    return ;
                }
                data_len = 0;
                while(true){
                    c = fgetc(fin);
                    if(feof(fin))
                        break;
                    buffer[data_len++] = c;
                    if(data_len >= BUFFER_SIZE){
                        send(client, buffer, data_len, 0);
                        data_len = 0;
                    }
                }
                if(data_len)
                    send(client, buffer, data_len, 0);
                else
                    send(client, buffer, 1, 0);
                fclose(fin);
                remove(tmpbuffer);
                printf("Send file success!\n");
            }
        }
    }
    close(server);
    return 0;
}
 
