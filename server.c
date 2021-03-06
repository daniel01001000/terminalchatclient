#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <netinet/in.h>

#include <string.h>

int main( int argc, char *argv[] ) {
   int sockfd, newsockfd, portno, clilen;
   char buffer[256];
   struct sockaddr_in serv_addr, cli_addr;
   int  n;
   int quitsign = 0; /* default 0, 1 = quit */
   
   /* First call to socket() function */
   sockfd = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd < 0) {
      perror("ERROR opening socket");
      exit(1);
   }
   
   /* Initialize socket structure */
   bzero((char *) &serv_addr, sizeof(serv_addr));
   portno = atoi(argv[1]);
   
   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno);
   
   /* Now bind the host address using bind() call.*/
   if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }
      
   /* Now start listening for the clients, here process will
      * go in sleep mode and will wait for the incoming connection
   */
   
   listen(sockfd,5);
   clilen = sizeof(cli_addr);
   
   /* Accept actual connection from the client */
   newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen);
   
   if (newsockfd < 0) {
      perror("ERROR on accept");
      exit(1);
   }
   
   /* If connection is established then start communicating */
   
   while(!quitsign){
      bzero(buffer,256);
      n = read( newsockfd,buffer,255 );
      
      if (n < 0) {
         perror("ERROR reading from socket");
         exit(1);
      }
      
      /* if other user quits, you also quit program */
      if (buffer[0]=='q' && buffer[1]=='\n'){
         quitsign = 1;
         strcpy(buffer, "user has left the chat");
         exit(0);
      }

      printf("Here is the message: %s\n",buffer);
      
      /* Write a response to the client */

      bzero(buffer,256);
      n = read(STDIN_FILENO, buffer, 255);

      /* if I decide to quit, send signal to quit and then quit
      myself */
      if (buffer[0]=='q' && buffer[1]=='\n'){
         quitsign = 1;
         strcpy(buffer, "user has left the chat");
      }
      n = write(newsockfd,buffer,255);
      
      /* quit after sending message of 'user has left the chat' to other
      user */
      if (quitsign){
         exit(0);
      }
      
      if (n < 0) {
         perror("ERROR writing to socket");
         exit(1);
      }
   }
   
      
   return 0;
}
