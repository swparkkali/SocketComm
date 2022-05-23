#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>

#define KPP_BUF_SIZE 80
#define KPP_PORT 9998
#define KPP_SA struct sockaddr
#define KPP_TRUE 1
#define KPP_FALSE 0

typedef struct
{
   int age;
   char name[12];
   char gender; // F or M
} KPP_PERSON;

typedef struct
{
   char* ip;
   int port;
} KPP_CLIENT;

typedef struct
{
   char name[12];
   int employee_num;
   char ceo_name[12];
} KPP_COMPANY;

char* getting_IP();
void error_handling(char *message);
void signal_handler(int signo);
void func(int clnt_sock);

static int clnt_sock_fd;
char buff[KPP_BUF_SIZE];

int main(int argc, char **argv)
{
   KPP_PERSON person;
   person.age = 29;
   strcpy(person.name, "SwPark");
   person.gender = 'M';

   KPP_CLIENT client;
   client.ip = getting_IP();
   client.port = 0000;

   KPP_COMPANY company;
   strcpy(company.name, "KAON");
   company.employee_num = 450;
   strcpy(company.ceo_name, "TEST");

	signal(SIGINT, signal_handler);
   memset(&buff, 0x00, KPP_BUF_SIZE);
	int clnt_sock, str_len;

	clnt_sock = socket(AF_INET, SOCK_STREAM, 0); // socket created.
	clnt_sock_fd = clnt_sock;	// copy to flag variable to close after.

	if (clnt_sock == -1) 
		error_handling("Socket creation failed...");
	else
		printf("Socket created Successfully.\n");

	struct sockaddr_in servaddr;
	memset(&servaddr, 0x00, sizeof(servaddr));
   //bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	servaddr.sin_port = htons(KPP_PORT);
   
	// connect the client socket to server socket
	if (connect(clnt_sock, (KPP_SA*)&servaddr, sizeof(servaddr)) != 0) 
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
   char buff[KPP_BUF_SIZE];

	while(1) 
   {
      memset(&buff, 0x00, KPP_BUF_SIZE);
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
         write(clnt_sock, buff, KPP_BUF_SIZE);
         printf("Server: %s\n", buff);
      }
      int str_len = read(clnt_sock, buff, KPP_BUF_SIZE);
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
		close(clnt_sock_fd);
		printf("\nClose socket.\n");
		exit(1);
	}
}

char* getting_IP()
{
   FILE *fp;
   const char *ip = "/sbin/ifconfig | grep inet | awk '{print $2}' | head -n 1";
   static char buf[KPP_BUF_SIZE];

   memset(buf, 0x00, KPP_BUF_SIZE);
   fp = popen(ip, "r");

   if(fp != NULL)
      while(fgets(buf, KPP_BUF_SIZE, fp) != NULL);

   pclose(fp);
   return buf;
}
