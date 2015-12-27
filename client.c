#include <stdio.h>
#include <stdlib.h>

#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

int main(int argc, char *argv[]) {
   int sockfd, portno, n;
   struct sockaddr_in serv_addr;
   struct hostent *server;
  
   char in_buffer[256];
   char out_buffer[256];
   int quitsign = 0;
   
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
      bzero(out_buffer,256);
      fgets(out_buffer,255,stdin);
   
      /* if user enter's letter q, quit program */
      if (out_buffer[0]=='q' && out_buffer[1]=='\n'){
         quitsign = 1;
         strcpy(out_buffer, "user has left the chat");
      }

      /* Send message to the server */
      n = write(sockfd, out_buffer, strlen(out_buffer));
      
      if (quitsign){
         exit(0);
      }

      if (n < 0) {
         perror("ERROR writing to socket");
         exit(1);
      }
      
      /* Now read server response */
      bzero(in_buffer,256);
      n = read(sockfd, in_buffer, 255);
      
      if (n < 0) {
         perror("ERROR reading from socket");
         exit(1);
      }

      if ( !strcmp(in_buffer, "user has left the chat") ) {
         exit(0);
      }

      printf("%s\n",in_buffer);
      
      
   }
   
   return 0;
}