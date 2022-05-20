#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/select.h>

#define CLNT_NUM 5
#define BUF_SIZE 100
#define PORT 9998
#define TRUE 1
#define FALSE 0

void error_handler(char* message);
void signal_handler(int signo);

static int serv_sock_fd;
static int clnt_sock_fd;
static int Reuse_addr_used = TRUE;

typedef struct client
{
    int fd;   // input their fild descripter. 
    bool used; // TRUE or FALSE
} KPP_CLIENT;

int main(int argc, char *argv[])
{
    signal(SIGINT, signal_handler);
    struct sockaddr_in serv_adr = {};
    int serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(PORT);

    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &Reuse_addr_used, sizeof(Reuse_addr_used));
    
    if (bind(serv_sock, (struct sockaddr*) &serv_adr, sizeof(serv_adr)) == -1)
        error_handler("bind() error");
    else
       printf("Socket binded Successfully.\n");


    if (listen(serv_sock, 5) == -1)
        error_handler("listen()");
    else
       printf("Listening...\n\n");

    // server socket is get ready.
    
    
    fd_set reads, cpy_reads;
    FD_ZERO(&reads);
    FD_SET(serv_sock, &reads);
    struct timeval timeout;
    
    KPP_CLIENT clnt_list[CLNT_NUM];
    memset(&clnt_list, 0x00, sizeof(clnt_list));
    
    char buf[BUF_SIZE];
    int fd_max = serv_sock;
    int fd_num;
    
    while(1) // Falling for checking data in realtime.
    {
        cpy_reads = reads;
        timeout.tv_sec = 30;
        timeout.tv_usec = 0000;

        if ((fd_num = select(fd_max + 1, &cpy_reads, 0, 0 , &timeout)) == -1) // if  failed
            break;
        
        if (fd_num == 0) // timed out
        {
            printf("Socket timed out, reconnection..\n");
            continue;
        }
              
        for(int i = 0; i < fd_num; i++)
        {
           if (FD_ISSET(serv_sock, &cpy_reads))
           {
              struct sockaddr_in clnt_adr = {};
              socklen_t adr_sz = sizeof(clnt_adr);

              for (int idx = 0 ; idx < CLNT_NUM; idx++)
              {
                 if (clnt_list[idx].used == FALSE)
                 {
                    int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &adr_sz);
                    FD_SET(clnt_sock, &reads);
                    clnt_list[idx].fd = clnt_sock;
                    clnt_list[idx].used = TRUE;

                    if (fd_max < clnt_sock)
                       fd_max = clnt_sock;

                    printf("** FD-%d Client connected. **\n", clnt_sock);
                    for(int idx = 0; idx < CLNT_NUM; idx++)
                       printf("Client[%d].fd = %d\t used = %d\n", idx, clnt_list[idx].fd, clnt_list[idx].used);
                    break;
                 }
              }
           }
           else
           {
              for(int idx = 0; idx < CLNT_NUM; idx++)
              {
                 if (FD_ISSET(clnt_list[idx].fd, &cpy_reads))
                 {
                    int str_len = read(clnt_list[idx].fd, buf, BUF_SIZE);
                    if (str_len == 0)
                    {
                       FD_CLR(clnt_list[idx].fd, &reads);
                       clnt_list[idx].used = FALSE;

                       printf("** FD-%d Client disconnected. **\n", clnt_list[idx].fd);
                       close(clnt_list[idx].fd);
                         
                       for(int idx = 0; idx < CLNT_NUM; idx++)
                          printf("Client[%d].fd = %d\t used = %d\n", idx, clnt_list[idx].fd, clnt_list[idx].used);
                    }
                    else
                    {
                       write(clnt_list[idx].fd, buf, str_len);
                       printf("FD-%d client said : %s", clnt_list[idx].fd, buf);
                    }
                 }
              }        
           }
        }
     }
    close(serv_sock);
    return 0;
}

void error_handler(char* message)
{
    fputs(message, stderr);
    fputc('\n', stderr);
    exit(1);
}

void signal_handler(int signo)
{
   const char *message = "Server has closed. Connection out.\n";

   if (signo == SIGINT)
   {
      printf("\n\nSIGINT raised..\n");
      close(serv_sock_fd);
      close(clnt_sock_fd);
      printf("Socket Closed..\n");
      printf("Terminated..\n");
      exit(1);
   }
}
