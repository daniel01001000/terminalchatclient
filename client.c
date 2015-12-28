#include <stdio.h>
#include <stdlib.h>
#include <netdb.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

typedef struct {
   struct sockaddr_in *socky;
   int *sockyfd;  
} Addys;


/* This function recieves messages from other user until
either I quit or other user sends quit sign */
void * reciever( void *socket_addy ) {
   
   char buffer[256];

   int sockfd, ret;
   int *sock_ptr;
   int clilen;
   int newsockfd;
   int quitsign = 0;
   char *quitptr;
   Addys * s_addy;
   
   s_addy = (Addys *)socket_addy;
   /* sock_ptr = (int*) socket_ptr;*/
   sockfd = *(s_addy->sockyfd);
   
   listen(sockfd,5);
   clilen = sizeof(*(s_addy->socky));
  
   /* Accept actual connection from the client */
   newsockfd = accept(sockfd, (struct sockaddr *)(s_addy->socky), &clilen);
  
   if (newsockfd < 0) {
      perror("ERROR on accept");
      exit(1);
   }


   /* while I haven't quit or other user hasn't sent a quit sign 
   continue receiving */
   while(!quitsign) {
      /* initialize buffer to 0s*/
      memset(buffer, 0, 256);

      /* recvfrom returns length of message in bytes; return negative
      if failure */
      /* ret = recvfrom(sockfd, buffer, 256, 0, NULL, NULL); */
      
      ret = read(newsockfd, buffer, 256);


      /*check for recvfrom failure*/
      if (ret<0) {
         printf("error recieving data");
      }


      /* check if string has termination msg */
      quitptr = strstr(buffer, "user has left the chat\n");
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

   
void * caller( void *socket_addy ) {
	char buffer[256];
    int sockfd, portno, n, ret;
   	int quitsign = 0;
   	int connected = 0;
   	char *quitptr;
   
   	Addys * s_addy;

   	s_addy = (Addys *) socket_addy; 
   	sockfd = *(s_addy->sockyfd);
   	

  
   	/* continue writing out until I disconnect */
   	while(!quitsign){
   		memset(buffer, 0, 256);
      	fgets(buffer,255,stdin);
      	
      
      	/* Now connect to the server */
        if(strstr(buffer, "connect")){
        	

        	if (connect(sockfd, (struct sockaddr*)(s_addy->socky), sizeof(*(s_addy->socky))) < 0) {
            	perror("ERROR connecting");
            } else {
            	connected = 1;
               	printf("connected successfully on my end\n");
            }
         }      
   
      	/* if user enter's letter q, quit program */
      	if (buffer[0]=='q' && buffer[1]=='\n'){
      		quitsign = 1;
         	strcpy(buffer, "user has left the chat\n");
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
   
   int tread_retval, twrite_retval; 
   int sockfd_r, sockfd_w; 
   int portno_r, portno_w;

   pthread_t tread, twrite;
   
   struct sockaddr_in serv_addr, cli_addr;
   Addys serv_addy, cli_addy;
   struct hostent *server;
   
   /*server setup */   
   bzero((char *) &serv_addr, sizeof(serv_addr));

   if (argc < 3) {
      fprintf(stderr,"usage %s hostname port\n", argv[0]);
      exit(0);
   } 
   
   portno_r = atoi(argv[2]);
   portno_w = atoi(argv[3]);
   
      
   /* listen socket point */ 
   sockfd_r = socket(AF_INET, SOCK_STREAM, 0);
   
   if (sockfd_r < 0) {
      perror("ERROR opening socket");
      exit(1);
   }

   serv_addr.sin_family = AF_INET;
   serv_addr.sin_addr.s_addr = INADDR_ANY;
   serv_addr.sin_port = htons(portno_r); 

   if (bind(sockfd_r, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
       perror("ERROR on binding");
       exit(1);
   }
   serv_addy.socky = &serv_addr;
   serv_addy.sockyfd = &sockfd_r;

   /* Write socket points */
   bzero((char *) &cli_addr, sizeof(cli_addr));
   
   server = gethostbyname(argv[1]);
   
   if (server == NULL) {
      fprintf(stderr,"ERROR, no such host\n");
      exit(0);
   }

   sockfd_w = socket(AF_INET, SOCK_STREAM, 0);

   if (sockfd_w < 0) {
      perror("ERROR opening socket");
      exit(1);
   }

   cli_addr.sin_family = AF_INET;
   bcopy((char *)server->h_addr, (char *)&cli_addr.sin_addr.s_addr, server->h_length);
   cli_addr.sin_port = htons(portno_w);
   
   
   cli_addy.socky = &cli_addr;
   cli_addy.sockyfd = &sockfd_w;


     
   /* create read thread, return 0 on success*/
   tread_retval = pthread_create(&tread, NULL, reciever, &serv_addy);

   /* if thread failed, tread_retval will have non-zero val */
   if (tread_retval) {
      printf("error creating read thread");
      exit(1);
   }

   /* create write thread, return 0 on success*/
   twrite_retval = pthread_create(&twrite, NULL, caller, &cli_addy);

   /* if thread failed, tread_retval will have non-zero val */
   if (twrite_retval) {
      printf("error creating write thread");
      exit(1);
   }     
    
   
   //wait for threads to finish
   pthread_join(tread  ,NULL);
   pthread_join(twrite  ,NULL);
   
   
   return 0;
}
