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

#define KPP_CLNT_NUM 5
#define KPP_BUF_SIZE 100
#define KPP_PORT 9998
#define KPP_SA struct sockaddr
#define KPP_TRUE 1
#define KPP_FALSE 0

void error_handler(char* message);
void signal_handler(int signo);

static int Serv_sock_fd;
static int Clnt_sock_fd;
static int Reuse_addr_used = KPP_TRUE;
static int Connect_clnt_cnt = 0;

typedef struct client
{
    int fd;   // input their fild descripter. 
    bool used; // KPP_TRUE or KPP_FALSE
} KPP_CLIENT;

int main(int argc, char *argv[])
{
    int Reuse_addr_used = KPP_TRUE;

    struct sockaddr_in serv_adr = {};
    int serv_sock = socket(PF_INET, SOCK_STREAM, 0);
    memset(&serv_adr, 0, sizeof(serv_adr));
    serv_adr.sin_family = AF_INET;
    serv_adr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_adr.sin_port = htons(KPP_PORT);

    signal(SIGINT, signal_handler);
    setsockopt(serv_sock, SOL_SOCKET, SO_REUSEADDR, &Reuse_addr_used, sizeof(Reuse_addr_used));
    
    if (bind(serv_sock, (KPP_SA*) &serv_adr, sizeof(serv_adr)) == -1)
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
    
    KPP_CLIENT clnt_list[KPP_CLNT_NUM];
    memset(&clnt_list, 0x00, sizeof(clnt_list));
    
    char buf[KPP_BUF_SIZE];
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
              socklen_t clnt_adr_sz = sizeof(clnt_adr);

              int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_adr, &clnt_adr_sz);

              for (int idx = 0 ; idx < KPP_CLNT_NUM; idx++)
              {
                 if (clnt_list[idx].used == KPP_FALSE)
                 {
                    FD_SET(clnt_sock, &reads);
                    clnt_list[idx].fd = clnt_sock;
                    clnt_list[idx].used = KPP_TRUE;

                    if (fd_max < clnt_sock)
                       fd_max = clnt_sock;

                    printf("** FD-%d Client connected. **\n", clnt_sock);
                    
                    //getsockname(clnt_sock, (KPP_SA*)&clnt_adr, &clnt_adr_sz);
                    //write(clnt_sock, ntohs(clnt_adr.sin_port), sizeof(ntohs(clnt_adr.sin_port)));
                    printf("[%d:%s:%u] > \n", clnt_sock, inet_ntoa(clnt_adr.sin_addr), ntohs(clnt_adr.sin_port));

                    for(int idx = 0; idx < KPP_CLNT_NUM; idx++)
                       printf("Client[%d].fd = %d\t used = %d\n", idx, clnt_list[idx].fd, clnt_list[idx].used);
                    printf("\n");
                    break;
                 }
              }

              Connect_clnt_cnt++;
              if (Connect_clnt_cnt > KPP_CLNT_NUM)
              {
                 const char *message = "Client request a connection, but queue is full.. drop the connection.";
                 printf("%s\n", message);
                 write(clnt_sock, message, sizeof(message));
                 close(clnt_sock);
                 Connect_clnt_cnt--;
              }
           }
           else
           {
              for(int idx = 0; idx < KPP_CLNT_NUM; idx++)
              {
                 if (FD_ISSET(clnt_list[idx].fd, &cpy_reads))
                 {
                    int str_len = read(clnt_list[idx].fd, buf, KPP_BUF_SIZE);
                    if (str_len == 0)
                    {
                       FD_CLR(clnt_list[idx].fd, &reads);
                       clnt_list[idx].used = KPP_FALSE;
                       Connect_clnt_cnt--;

                       printf("** FD-%d Client disconnected. **\n", clnt_list[idx].fd);
                       close(clnt_list[idx].fd);
                         
                       for(int idx = 0; idx < KPP_CLNT_NUM; idx++)
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
      close(Serv_sock_fd);
      close(Clnt_sock_fd);
      printf("Socket Closed..\n");
      printf("Terminated..\n");
      exit(1);
   }
}
