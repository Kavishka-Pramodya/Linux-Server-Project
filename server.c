#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <signal.h>

int main(){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    
    signal(SIGCHLD, SIG_IGN);//block zombie process

    int opt = 1;
    setsockopt(server_fd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in serv_addr; 
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(8080);

    if(bind(server_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr))==0){
        printf("Attached IP and Port for the Socket.....\n");
    }else{//-1
        perror("Attaching Failed.....\n");
        exit(1);
    };
    
    if(listen(server_fd,10)==0){
        printf("Listening.....\n");
    }else{//-1
        perror("Listen Failed.....\n");
        exit(1);
    };
    while(1){
    
    struct sockaddr_in cli_addr;
    int len = sizeof(cli_addr);
    int new = accept(server_fd,(struct sockaddr*)&cli_addr,&len);
    
    if(new>0){
        pid_t pid = fork();

        if(pid == 0){
            close(server_fd);
        
            char recv_msg[1024] = {0};
            int recv_message = recv(new,recv_msg,1024,0);
        //
            if(recv_message>0){
                char method[10],filename[100],protocol[20];
                sscanf(recv_msg, "%s %s %s",method,filename,protocol);
                printf("Client Request:%s\n",recv_msg);
                printf("method:%s\n",method);
                printf("filename:%s\n",filename);
                printf("protocol:%s\n\n\n",protocol);


                char *file_to_open;
                char *content_type = "text/html";

        //printf("%s",file_to_open);
                if(strcmp(filename,"/")==0){
                    file_to_open = "index.html";
                }else{
                    file_to_open = filename+1;//this line remove "/" in /index.html
                }
                if(strstr(file_to_open, ".css")) content_type = "text/css";
                else if(strstr(file_to_open, ".jpg")) content_type = "image/jpeg";
                else if(strstr(file_to_open, ".png")) content_type = "image/png";
                else if(strstr(file_to_open, ".js")) content_type = "application/javascript";
        
                FILE *file = fopen(file_to_open, "rb");
                if(file == NULL){
                    char *error = "HTTP/1.1 404 Not Found\r\nContent-Type: text/html\r\n\r\n<h1 style='text-align:center;margin-top:100px;font-size:100px;background-color:#f4f4f4;'>404 Not Found</h1>";
                    send(new,error,strlen(error),0);
                }else{
                    char header[200];
                    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s\r\n\r\n", content_type);
                    send(new,header,strlen(header),0);

                    unsigned char file_buffer[1024];
                    size_t bytes_read;
                    while((bytes_read = fread(file_buffer,1,sizeof(file_buffer),file))>0){
                        send(new,file_buffer,bytes_read,0);
                    }
                    fclose(file);
                }
            }
            close(new);
            exit(0);
        }else if(pid > 0){
            close(new);
        }else{
            perror("Fork failed");
            close(new);
        }
    
    }
    }
    return 0;
}