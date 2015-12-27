#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
  
   char buffer[256];
   int quitsign = 0;
   char *quitptr;
   
   if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   } 
	
   portno = atoi(argv[2]);
   
   /* Create a socket point */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
	
   server = gethostbyname(argv[1]);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }
   
   bzero((char *) &serv_addr, sizeof(serv_addr));
   serv_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
   serv_addr.sin_port = htons(portno);
   
   /* Now connect to the server */
   if (connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR connecting");
      exit(1);
   }
   
   /* Now ask for a message from the user, this message
      * will be read by server
   */
	
   printf("chat room connected\n ");
   while (!quitsign){
      bzero(buffer,256);
      fgets(buffer,255,stdin);
   
      /* if user enter's letter q, quit program */
      if (buffer[0]=='q' && buffer[1]=='\n'){
         quitsign = 1;
         strcpy(buffer, "user has left the chat");
      }

      /* Send message to the server */
      n = write(sockfd, buffer, strlen(buffer));
      
      if (quitsign){
         exit(0);
      }

      if (n < 0) {
         perror("ERROR writing to socket");
         exit(1);
      }
      
      /* Now read server response */
      bzero(buffer,256);
      n = read(sockfd, buffer, 255);
      
      /* check if string has termination msg */
      quitptr = strstr(buffer, "user has left the chat");
      if (quitptr!=NULL && buffer[22]=='\0'){
         quitsign = 1;
      }

      if (n < 0) {
         perror("ERROR reading from socket");
         exit(1);
      }

      if ( !strcmp(buffer, "user has left the chat") ) {
         exit(0);
      }

      printf("%s\n",buffer);
      
      /* will exit if other user disconnects before I do */
      if(quitsign){
         exit(0);
      }
      
   }
   
   return 0;
}