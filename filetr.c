#include<stdio.h>
#include<winsock2.h>
#include<string.h>
//#include<ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#define MAX_USER 10

typedef struct {
    int socket_fd;
    char buffer[4096];
    int data_sent;
    char filename[40];
    int header_on;
    int filesize;
    char method[10];
    FILE *fp;
    
    
}user_info;

int main(){
    WSADATA wsa;
    WSAStartup(MAKEWORD(2,2),&wsa);

    int server_socket = socket(AF_INET,SOCK_STREAM,0);
    if(server_socket== INVALID_SOCKET){
        printf("Socket error %d\n",WSAGetLastError());
        return -1;
    }
    // int off =0;
    // setsockopt(server_socket,IPPROTO_IPV4,0,(char*)&off,sizeof(off));

    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(5001);
    server_addr.sin_addr.S_un.S_addr = INADDR_ANY;

    if(bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr))){
        printf("Bind Error %d\n",WSAGetLastError());
        return -1;
    }

    listen(server_socket,MAX_USER);
    printf("Listenning on port 5001");

    fd_set readfd,writefd;
    user_info* data = calloc(MAX_USER,sizeof(user_info));
    while(1){
        FD_ZERO(&readfd);
        FD_ZERO(&writefd);
        FD_SET(server_socket,&readfd);
        for(int i=0;i<MAX_USER;i++){
            int s=data[i].socket_fd;
            if(s>0){
                FD_SET(s,&readfd);
                printf("user readfd %d",i);
            }
            if(s>0 && (data[i].header_on == 0 || data[i].data_sent < data[i].filesize)){
                FD_SET(s,&writefd);
                printf("user writefd %d",i);
            }

        }
        int activity = select(0,&readfd,&writefd,NULL,NULL);
        if(FD_ISSET(server_socket,&readfd)){
            int c_socket = accept(server_socket,NULL,NULL);

            if(c_socket==SOCKET_ERROR){
                printf("connection error!");
                closesocket(c_socket);
                continue;
            }
            else{
                printf("new connection \n");
            }
            for(int i=0;i<MAX_USER;i++){
                int s = data[i].socket_fd;
                if(s>0){
                    continue;
                }
                else{
                    printf("newuser set %d set",i);
                    data[i].data_sent=0;
                    memset(&data[i],0,sizeof(user_info));
                    data[i].fp=NULL;
                    data[i].socket_fd=c_socket;
                    break;
                }
            }

        }
        printf("read fd block forward : ");
        for(int i=0;i<MAX_USER;i++){
            int s=data[i].socket_fd;
            char recv_buffer[4096];
            if(s>0 && FD_ISSET(s,&readfd)){
                int rec_byte = recv(s,recv_buffer,4095,0);
                printf("receive bytes : %d",rec_byte);
                if(rec_byte ==0){
                    printf("connection closed\n");
                    data[i].socket_fd=0;
                    memset(&data[i],0,sizeof(user_info));
                    data[i].fp=NULL;
                    // data[i].pending_length=0;
                }
                else if(rec_byte<0){
                     int err = WSAGetLastError();

                        if(err == WSAEWOULDBLOCK){
                            printf("normal error in case of select");
                            continue;
                        } else {
                            printf("data not found \n");
                            data[i].socket_fd=0;
                            memset(&data[i],0,sizeof(user_info));
                            data[i].fp=NULL;
                        }
                    
                    // data[i].pending_length=0;
                }
                else{
                    recv_buffer[rec_byte]='\0';
                    printf("Request by user %d : %s\n",i,recv_buffer);
                //     char version[200];
                //     sscanf(recv_buffer, "%s /%s %s", data[i].method, data[i].filename, version);
                //     printf("Parsed file: %s\n", data[i].filename);
                //     if (strlen(data[i].filename) == 0) {
                //         strcpy(data[i].filename, "index.html"); // serve a default file

                //         printf("It is index.html part ");
                //     }
                //     if(strcmp(data[i].filename, "favicon.ico") == 0){
                //         closesocket(s);
                //         memset(&data[i],0,sizeof(user_info));
                //         data[i].fp=NULL;
                //         printf("it is favicon.io part ");
                //         continue;
                //     }
                //     data[i].data_sent=0;
                //     data[i].header_on=0;
                //     data[i].fp=NULL;
                //     data[i].data_sent=0;
                //     data[i].header_on=0;
                //     data[i].filesize=0;
                    char url[256];
                    char version[32];
                    int parsed =sscanf(recv_buffer,"%s %s %s",data[i].method,url,version);
                    if(parsed<2){
                        printf("Bad request\n");
                        closesocket(s);
                        memset(&data[i],0,sizeof(user_info));
                        continue;
                    }
                    if(url[0]=='/'){
                        strcpy(data[i].filename,url+1);
                    } else {
                        strcpy(data[i].filename,url);
                    }
                    if(strlen(data[i].filename)==0){
                        strcpy(data[i].filename,"index.html");
                    }

                    printf("Parsed file: [%s]\n",data[i].filename);
                }
            }
            // free(recv_buffer);

            
        }
        printf("writefd block");
        for(int i=0;i<MAX_USER;i++){
                int s = data[i].socket_fd;
                if(s>0 && FD_ISSET(s,&writefd)){
                    char header[512];
                    if(data[i].fp==NULL){
                        data[i].fp = fopen(data[i].filename,"rb");
                        if(!data[i].fp){
                            
                            closesocket(s);
                            memset(&data[i],0,sizeof(user_info));
                            data[i].fp=NULL;
                            printf("file not openning error");
                            continue;
                        }
                        fseek(data[i].fp,0,SEEK_END);
                        data[i].filesize=ftell(data[i].fp);
                        fseek(data[i].fp,0,SEEK_SET);
                    }

                    FILE *fcur= data[i].fp;
                    
                    
                    
                   
                    //fseek(fcur,data[i].data_sent,SEEK_SET);it is unneccary as file pointer already moves 
                    sprintf(header,
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: application/octet-stream\r\n"
                        "Content-Length: %d\r\n"
                        "Content-Disposition: attachment; filename=\"%s\"\r\n"
                        "Connection: close\r\n"
                        "\r\n",
                        data[i].filesize,
                        data[i].filename
                    );
                    printf("filename: %s filesize: %d",data[i].filename,data[i].filesize);
                    if(data[i].header_on==0){
                        int sent = send(s, header, strlen(header), 0);

                            if(sent <= 0){
                                closesocket(s);
                                memset(&data[i],0,sizeof(user_info));
                                printf("header not sent");
                                continue;
                            }

                            data[i].header_on=1;
                            printf("sent : %d  header : %s",sent,header);
                            continue;
                    }
                    else{
                        
                        // if(bytes>0){
                        //     send(s, data[i].buffer, bytes, 0);
                        //     data[i].data_sent+=bytes;
                        // }
                        
                            int bytes = fread(data[i].buffer,1,sizeof(data[i].buffer),fcur);

                            if(bytes > 0){
                                int sent = send(s, data[i].buffer, bytes, 0);

                                if(sent <= 0){
                                    closesocket(s);
                                    memset(&data[i],0,sizeof(user_info));
                                    printf("data not sent ");
                                    continue;
                                }

                                data[i].data_sent += sent;
                                fseek(fcur, sent ,SEEK_CUR-bytes);
                                printf("sent : %d bytes : %d ",sent,bytes);
                            
                            }
                    }
                    // int bytes;
                    // while ((bytes = fread(data[i].buffer,1, sizeof(data[i].buffer), fp) )> 0) {
                        
                    // }
                    // if(data[i].data_sent >= data[i].filesize && data[i].chunk_sent == data[i].chunk_size){
                    //     fclose(data[i].fp);
                    //     closesocket(s);
                    //     memset(&data[i],0,sizeof(user_info));
                    // }
                    printf("data sent checking block \n");
                    if(data[i].data_sent >= data[i].filesize){
                        fclose(data[i].fp);
                        closesocket(s);
                        memset(&data[i],0,sizeof(user_info));
                    }
                }

        }
    }
    

    WSACleanup();
    free(data);
    return 0;
}