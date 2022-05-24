#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <signal.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if.h>

#define KPP_BUF_SIZE 80
#define KPP_PORT 9998
#define KPP_SA struct sockaddr
#define KPP_TRUE 1
#define KPP_FALSE 0

typedef struct
{
   char name[12];
   int age;
   char gender[8]; // F or M
   int height;
} KPP_PERSON;

typedef struct
{
   char *ip;
   char server[10];
} KPP_CLIENT;

typedef struct
{
   char name[12];
   int employee_num;
   char ceo_name[12];
} KPP_COMPANY;


void error_handling(char *message);
void signal_handler(int signo);
void func(int clnt_sock, KPP_PERSON *person, KPP_CLIENT *client);
char* getting_ip();

static int Clnt_sock_fd;
char buff[KPP_BUF_SIZE];

int main(int argc, char **argv)
{   
   KPP_PERSON person;
   strcpy(person.name, "SwPark");
   strcpy(person.gender, "Male");
   person.age = 29;
   person.height = 100;

   KPP_CLIENT client;
   client.ip = getting_ip();
   strcpy(client.server, "Ubuntu");
    
	signal(SIGINT, signal_handler);
   memset(&buff, 0x00, KPP_BUF_SIZE);
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
	servaddr.sin_port = htons(KPP_PORT);
   
	// connect the client socket to server socket
	if (connect(clnt_sock, (KPP_SA*)&servaddr, sizeof(servaddr)) != 0) 
		error_handling("Connection with the server failed...");
	else
		printf("Connected to the server..\n");

	func(clnt_sock, &person, &client); // call the fuction to mutual interaction.
   close(clnt_sock);

	return 0;
}

void func(int clnt_sock, KPP_PERSON *person, KPP_CLIENT *client)
{
   char buff[KPP_BUF_SIZE];
   int n; 
   
   memset(buff, 0x00, KPP_BUF_SIZE);
   memcpy(buff, person, sizeof(KPP_PERSON));

   printf("buff = %s\n", buff);
	while(1) 
   {
      if ((strncmp(buff, "exit", 4)) == 0) 
      {
			printf("Client Exit...\n");
         close(clnt_sock);
         exit(1);
			break;
		}
      else
      {
         KPP_PERSON* tmpPerson = (KPP_PERSON*)buff;
         write(clnt_sock, buff, sizeof(KPP_PERSON));
         printf("Server: %s\n", tmpPerson->name);
         printf("Server: %s\n", tmpPerson->gender);
         printf("Server: %d\n", tmpPerson->age);
         printf("Server: %d\n", tmpPerson->height);
         write(clnt_sock, tmpPerson, sizeof(tmpPerson));
      }

      if ((read(clnt_sock, buff, KPP_BUF_SIZE) <= 0)) // if server terminated.
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

char* getting_ip()
{
   struct ifreq ifr;
   static char ip[30];
   int s = socket(AF_INET, SOCK_DGRAM, 0);
   strncpy(ifr.ifr_name, "ens4", IFNAMSIZ);

   if (ioctl(s, SIOCGIFADDR, &ifr) < 0)
      error_handling("Get IP failed.\n");
   else
      inet_ntop(AF_INET, ifr.ifr_addr.sa_data + 2, ip, sizeof(struct sockaddr)); 

   return ip;
}
