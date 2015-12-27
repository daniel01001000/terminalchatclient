#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>


/* This function recieves messages from other user until
either I quit or other user sends quit sign */
void * reciever( void *socket_ptr ) {
   
   char buffer[256];

   int sockfd, ret;
   int *sock_ptr;
   int quitsign = 0;
   
   char *quitptr;

   sock_ptr = (int*) socket_ptr;
   sockfd = *sock_ptr;

   /* while I haven't quit or other user hasn't sent a quit sign 
   continue receiving */
   while(!quitsign) {
      /* initialize buffer to 0s*/
      memset(buffer, 0, 256);

      /* recvfrom returns length of message in bytes; return negative
      if failure */
      ret = recvfrom(sockfd, buffer, 256, 0, NULL, NULL);
      /*check for recvfrom failure*/
      if (ret<0) {
         printf("error recieving data");
      }


      /* check if string has termination msg */
      quitptr = strstr(buffer, "user has left the chat");
      if (quitptr!=NULL && buffer[22]=='\0'){
         quitsign = 1;
      }

      printf("other user: "); /* 'other user' will be username of other user for now */
      fputs(buffer, stdout); /* print out what I received from other user */

      /* if quitsign is nonzero, time to quit */
      if(quitsign){
         exit(0);
      }
   }
}

   
void * caller( struct sockaddr_in *serv_addy ) {


   char buffer[256];
   
   int sockfd, portno, n, ret;
   int quitsign = 0;

   char *quitptr;

   int connected = 0;
   
   sockfd = socket(AF_INET, SOCK_STREAM, 0);

  
   /* continue writing out until I disconnect */
   while(!quitsign){

      memset(buffer, 0, 256);
      fgets(buffer,255,stdin);

      /* keep attempting to connect */
      //while ( !connected ) {
         /* Now connect to the server */
         if(strstr(buffer, "connect")){
            if (connect(sockfd, (struct sockaddr*)serv_addy, sizeof(*serv_addy)) < 0) {
               perror("ERROR connecting");
            } else {
               connected = 1;
            }
         }
      //}

      
   
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
   }
}




int main(int argc, char *argv[]) {
   
   int tread_retval, twrite_retval, sockfd, portno;
   pthread_t tread, twrite;
   
   struct sockaddr_in serv_addr;
   struct hostent *server;

   struct sockaddr_in *serv_addy_ptr;
   
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
   
   /* set pointer to created serv struct */
   serv_addy_ptr = &serv_addr;
   

   /* create read thread, return 0 on success*/
   tread_retval = pthread_create(&tread, NULL, reciever, (void *) sockfd);

   /* if thread failed, tread_retval will have non-zero val */
   if (tread_retval) {
      printf("error creating read thread");
      exit(1);
   }

   /* create write thread, return 0 on success*/
   twrite_retval = pthread_create(&twrite, NULL, caller, serv_addy_ptr);

   /* if thread failed, tread_retval will have non-zero val */
   if (twrite_retval) {
      printf("error creating write thread");
      exit(1);
   }     
    
   
   return 0;
}