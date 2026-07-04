#include<stdio.h>
#include<stdlib.h>
#define MAX_USER 10
#include<sys/socket.h>
#include <sys/stat.h>
#include<netinet/in.h>
#include<unistd.h>
#include <errno.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include<string.h>

typedef struct {
    int socket_fd;
    char buffer[4096];
    int data_sent;
    int data_recieved;
    char filename[40];
    int header_on;
    int filesize;
    char method[10];
    int filesizerecieved;
    FILE *fp;
    
}user_info;
const char *get_mime_type(const char *filename) {
    const char *ext = strrchr(filename, '.');

    if (!ext) {
        return "application/octet-stream";
    }
    if (strcmp(ext, ".html") == 0) return "text/html";
    if (strcmp(ext, ".css") == 0) return "text/css";
    if (strcmp(ext, ".js") == 0) return "application/javascript";
    if (strcmp(ext, ".png") == 0) return "image/png";
    if (strcmp(ext, ".jpg") == 0) return "image/jpeg";
    if (strcmp(ext, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(ext, ".txt") == 0) return "text/plain";
    if (strcmp(ext, ".pdf") == 0) return "application/pdf";

    return "application/octet-stream";
}

int send_all(int s,const char *buf,int len) {
    int total = 0;

    while (total<len) {
        int sent=send(s,buf+total,len-total,0);
        if (sent<=0){
            return -1;
        }
        total += sent;
    }
    return total;
}

void send_simple_response(int s,const char *body) {
    char header[512];
    int body_len = strlen(body);
    int header_len = snprintf(
        header,
        sizeof(header),
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/plain\r\n"
        "Content-Length: %d\r\n"
        "Connection: close\r\n"
        "\r\n",
        body_len
    );
    send_all(s, header, header_len);
    send_all(s, body, body_len);
}

int get_content_length(const char *request) {
    char *cl = strcasestr(request, "Content-Length:");
    if (!cl) {
        return -1;
    }

    cl+= strlen("Content-Length:");
    while (*cl ==' ') {
        cl++;
    }
    return atoi(cl);
}

int is_safe_upload_name(const char *name) {
    if (name[0] == '\0') return 0;
    if (strstr(name, "..")) return 0;
    if (strchr(name, '/')) return 0;
    if (strchr(name, '\\')) return 0;

    return 1;
}
int handle_upload(int s, char *url, char *request, int rec_byte) {
    const char *prefix = "/upload/";
    if (strncmp(url,prefix,strlen(prefix)) != 0) {
        return 0; 
    }
    char *filename = url+strlen(prefix);
    if (!is_safe_upload_name(filename)){
        send_simple_response(s,"Invalid filename\n");
        return 1;
    }
    int content_length = get_content_length(request);
    if (content_length<0){
        send_simple_response(s,"Content-Length missing\n");
        return 1;
    }
    char *body_start = strstr(request, "\r\n\r\n");
    if (!body_start){
        send_simple_response(s,"Bad HTTP request\n");
        return 1;
    }
    body_start+=4;
    int header_size = body_start-request;
    int body_received = rec_byte-header_size;
    mkdir("uploads", 0755);
    char path[512];
    snprintf(path,sizeof(path),"uploads/%s",filename);
    FILE *fp = fopen(path,"wb");
    if (!fp){
        send_simple_response(s, "Failed to create file\n");
        return 1;
    }

    if (body_received > 0) {
        fwrite(body_start, 1, body_received, fp);
    }

    int remaining = content_length - body_received;
    char buf[4096];

    while (remaining>0){
        int to_read=remaining;
        if (to_read> sizeof(buf)){
            to_read =sizeof(buf);
        }
        int n= recv(s,buf,to_read,0);
        if (n<=0){
            fclose(fp);
            return 1;
        }
        fwrite(buf,1,n,fp);
        remaining-=n;
    }
    fclose(fp);
    send_simple_response(s,"Upload successful\n");
    return 1;
}

int main(){
    
    int server_socket = socket(AF_INET,SOCK_STREAM,0);
    if(server_socket== -1){
        printf("Socket error %d\n",errno);
        return -1;
    }
    // int off =0;
    // setsockopt(server_socket,IPPROTO_IPV4,0,(char*)&off,sizeof(off));

    struct sockaddr_in server_addr;
    memset(&server_addr,0,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(5001);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    int opt = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(bind(server_socket,(struct sockaddr*)&server_addr,sizeof(server_addr))){
        printf("Bind Error %d\n",errno);
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
        int maxfd = server_socket;
        for(int i=0;i<MAX_USER;i++){
            int s=data[i].socket_fd;
            if(s>0){
                FD_SET(s,&readfd);
                printf("user readfd %d",i);
            }
            if(s > 0 && data[i].filename[0] != '\0' &&
                (data[i].header_on == 0 || data[i].data_sent < data[i].filesize)) {
                FD_SET(s,&writefd);
                printf("user writefd %d",i);
            }
            if(s>maxfd){
                maxfd = s;
            }

        }
        int activity = select(maxfd +1,&readfd,&writefd,NULL,NULL);
        if(FD_ISSET(server_socket,&readfd)){
            int c_socket = accept(server_socket,NULL,NULL);

            if(c_socket==-1){
                printf("connection error!");
                close(c_socket);
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
                     int err = errno;

                        if (err == EAGAIN || err == EWOULDBLOCK){
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
                    // This is to prevent buffer overflows(stopping malicious actors from entering junk urls to fill in, beyond which they could input bad code) (leaving 1 byte for null terminator)
                    int parsed = sscanf(recv_buffer, "%9s %255s %31s", data[i].method, url, version);
                    if(parsed<2){
                        printf("Bad request\n");
                        close(s);
                        memset(&data[i],0,sizeof(user_info));
                        continue;
                    }
                    if (strcmp(data[i].method, "POST") == 0) {
                        int handled = handle_upload(s, url, recv_buffer, rec_byte);

                        if (handled) {
                            close(s);
                            memset(&data[i], 0, sizeof(user_info));
                            continue;
                        }
                    }
                    char size[4096];
                    if(url[0]=='/'){
                        strcpy(data[i].filename,url+1);
                    } else {
                        strcpy(data[i].filename,url);
                    }
                    if(strcmp(data[i].method,"POST")==0){
                        int parsed = sscanf(recv_buffer,"Content-Length:%s\r\n",size);
                        data[i].filesizerecieved = strtol(size,NULL,10);
                    }
                    if(strlen(data[i].filename)==0){
                        strcpy(data[i].filename,"public/index.html");
                    }


		    // Security Fix: Prevent Path Traversal
                    if(strstr(data[i].filename, "..") != NULL) {
                        printf("Security alert: Directory traversal attempt blocked from user %d\n", i);
                        close(s);
                        memset(&data[i], 0, sizeof(user_info));
                        continue;
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
			    char *not_found_header = "HTTP/1.1 404 Not Found\r\n"
                        	     "Content-Length: 0\r\n"
                        	     "Connection: close\r\n\r\n";
			    send(s, not_found_header, strlen(not_found_header), 0);
    
			    printf("404 Not Found sent for file: %s\n", data[i].filename);
			    close(s);
			    memset(&data[i], 0, sizeof(user_info));
			    continue;
			}
			fseek(data[i].fp,0,SEEK_END);
                        data[i].filesize=ftell(data[i].fp);
                        fseek(data[i].fp,0,SEEK_SET);
                    }

                    FILE *fcur= data[i].fp;
                    
                    
                    
                   
                    //fseek(fcur,data[i].data_sent,SEEK_SET);it is unneccary as file pointer already moves 
                    const char* mime = get_mime_type(data[i].filename);
                    sprintf(header,
                        "HTTP/1.1 200 OK\r\n"
                        "Content-Type: %s\r\n"
                        "Content-Length: %d\r\n"
                        // "Content-Disposition: attachment; filename=\"%s\"\r\n"
                        "Connection: close\r\n"
                        "\r\n",
                        mime,
                        data[i].filesize
                    );
                    printf("filename: %s filesize: %d",data[i].filename,data[i].filesize);
                    if(data[i].header_on==0){
                        int sent = send(s, header, strlen(header), 0);

                            if(sent <= 0){
                                close(s);
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
                                    close(s);
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
                        close(s);
                        memset(&data[i],0,sizeof(user_info));
                    }
                }

        }
    }
    

    free(data);
    return 0;
}
