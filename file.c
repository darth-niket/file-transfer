#include "file.h"
#include <stdio.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <pthread.h>
#define MAX 256
/*
 *  Here is the starting point for your netster part.2 definitions. Add the
 *  appropriate comment header as defined in the code formatting guidelines
 */

/* Add function definitions */
void file_server(char* iface, long port, int use_udp, FILE* fp) {
    int sock_conn;
    if(use_udp==1)
    {   
		printf("I am here\n");
		sock_conn = socket(AF_INET, SOCK_DGRAM, 0);
	}
    else
    { 
		sock_conn = socket(AF_INET, SOCK_STREAM, 0); 
	} 
	
    if (sock_conn < 0)
    {
        perror("Socket creation failed");
        exit(1);
    }
	printf("%d\n", use_udp);

    struct sockaddr_in servaddr;
    servaddr.sin_port = htons(port);
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;

    if((bind(sock_conn, (struct sockaddr*)&servaddr, sizeof(servaddr))) != 0)
    {
    printf("[-]Socket Binding Failed\n");
    exit(1);
    }

    int listentotcp;
    if(use_udp==0)
    {
      listentotcp = listen(sock_conn, 5);

      if(listentotcp < 0)
      {
          printf("Listen Failed.....");
          exit(1);
      }

      int x = 0; 
      
      while (1)
      {
          int client_sock;
          struct sockaddr_in cli_address;
          socklen_t addr_len = sizeof(cli_address);

          
          client_sock = accept(sock_conn, (struct sockaddr *)&cli_address, &addr_len);

          if (client_sock < 0)
          {
              printf("Accept failed....\n");
              exit(1);
          }
          x += 1;
          printf("connection %d from ('%s', %d)\n", x, inet_ntoa(cli_address.sin_addr), ntohs(cli_address.sin_port));
          char buffer[MAX];
          ssize_t bytes_read = 0;
          while((bytes_read = recv(client_sock, buffer, MAX,0)) > 0)
          {
          fwrite(buffer, sizeof(char), bytes_read, fp);
          if(ferror(fp))
          {
              printf("Error writing....\n");
              close(client_sock);
              fclose(fp);
              exit(1);
          }
          }
          fclose(fp);
          if(bytes_read == -1)
              printf("Error in server fp\\n");
          close(client_sock);
          exit(0);
      }
	}
	else
	{
    while(1)
    {
     char buffer[256];
     struct sockaddr_in cli_address;
     socklen_t address_length = sizeof(cli_address);
     ssize_t bytes_read = 0;
     while(((bytes_read = recvfrom(sock_conn, buffer, MAX, 0, (struct sockaddr *)&cli_address, &address_length))) > 0)
     {
     if (strcmp(buffer,"DONEDONEDONE")==0)
	 {
         fclose(fp);
         close(sock_conn);
         exit(0);
     }
     fwrite(buffer, sizeof(char), bytes_read, fp);
     bzero(buffer, sizeof(buffer));
     if(ferror(fp))
     {
         printf("Error writing ......\n");
         close(sock_conn);
         fclose(fp);
         exit(1);
     }
     
     }
     fclose(fp);
     close(sock_conn);
     exit(0);
     if(bytes_read == -1)
         printf("Error in server fp\n");
    }
    }
}

void file_client(char* host, long port, int use_udp, FILE* fp) {
    if (strcmp(host, "localhost") == 0)
    {
        host = "127.0.0.1";
    }
    int client_conn;
    if (use_udp)
    {   
        client_conn = socket(AF_INET, SOCK_DGRAM, 0); 
	}
    else
    {
		client_conn = socket(AF_INET, SOCK_STREAM, 0);  
	}
    if (client_conn < 0)
    {
        printf("Socket creation failed...");
        exit(1);
    }

    struct sockaddr_in servaddr;
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(host);



    if (use_udp == 0)
    {
        int conn = connect(client_conn, (struct sockaddr *)&servaddr, sizeof(servaddr));
        if (conn < 0)
        {
            perror("Connect failed");
            exit(1);
        }
    }
    char buffer[MAX];
    size_t bytes_read = 0;
    ssize_t bytes_written = 0;
    socklen_t addr_len = sizeof(servaddr);
    if(use_udp==0)
	{
     do{
         bytes_read = fread(buffer, sizeof(char), sizeof(buffer), fp);
         if(ferror(fp))
         {
             printf("Error reading from file.\n");
             close(client_conn);
             fclose(fp);
             exit(1);
         }
         send(client_conn, buffer,bytes_read,0);
         bzero(buffer, sizeof(buffer));
        } while(!feof(fp) && bytes_written != -1);
     fclose(fp);
     close(client_conn);
     exit(0);
	}
	else
	{
     do{
         bytes_read = fread(buffer, sizeof(char), sizeof(buffer), fp);
         if(ferror(fp))
         {
             printf("Error reading from file.\n");
             close(client_conn);
             fclose(fp);
             exit(1);
         }
         sendto(client_conn, buffer, bytes_read, 0, (struct sockaddr *)&servaddr, addr_len);
         bzero(buffer, sizeof(buffer));
        } while(!feof(fp) && bytes_written != -1);
     strcpy(buffer,"DONEDONEDONE");
     sendto(client_conn, buffer, 256, 0, (struct sockaddr *)&servaddr, addr_len);
     bzero(buffer, sizeof(buffer));
     fclose(fp);
     close(client_conn);
     exit(0);
    }  
}
