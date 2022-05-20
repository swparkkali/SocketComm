#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>

#define BUF_SIZE 80
#define PORT 9998
#define SA struct sockaddr
#define TRUE 1
#define FALSE 0

void error_handling(char *message);
void signal_handler(int signo);
void func(int clnt_sock);

static int Clnt_sock_fd;
char buff[BUF_SIZE];

int main(int argc, char **argv)
{
	signal(SIGINT, signal_handler);
   memset(&buff, 0x00, BUF_SIZE);
	int clnt_sock, str_len;

	clnt_sock = socket(AF_INET, SOCK_STREAM, 0); // socket created.
	Clnt_sock_fd = clnt_sock;	// copy to flag variable to close after.

	if (clnt_sock == -1) 
		error_handling("Socket creation failed...");
	else
		printf("Socket created Successfully.\n");

	struct sockaddr_in servaddr;
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(PORT);
   
	// connect the client socket to server socket
	if (connect(clnt_sock, (SA*)&servaddr, sizeof(servaddr)) != 0) 
		error_handling("Connection with the server failed...");
	else
		printf("Connected to the server..\n");

	func(clnt_sock); // call the fuction to mutual interaction.
   close(clnt_sock);

	return 0;
}

void func(int clnt_sock)
{
   int n;
   char buff[BUF_SIZE];

	while(1) 
   {
      memset(&buff, 0x00, BUF_SIZE);
		printf("Client: ");
		n = 0;
		while ((buff[n++] = getchar()) != '\n');
     	
      if ((strncmp(buff, "exit", 4)) == 0) 
      {
			printf("Client Exit...\n");
         close(clnt_sock);
			break;
		}
      else
      {
         write(clnt_sock, buff, BUF_SIZE);
         printf("Server: %s\n", buff);
      }
      int str_len = read(clnt_sock, buff, BUF_SIZE);
      if ((str_len <= 0)) // if server terminated.
      {
		   printf("Server has closed.\n");
         exit(1);
      }
   }
}

void error_handling(char* message)
{
   fputs(message, stderr);
   fputc('\n', stderr);
   exit(1);
}

void signal_handler(int signo)
{
	if (signo == SIGINT)
	{
		printf("\nSIGINT catched.\n");
		close(Clnt_sock_fd);
		printf("\nClose socket.\n");
		exit(1);
	}
}
