/**
Copyright (c) 2016
*/
/* A simple server in the internet domain using TCP
The port number is passed as an argument */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>

#define CONN_SIZE 15
#define REPO_SIZE 256

void error(const char *msg)
{
    perror(msg);
    exit(1);
}

char *repo[REPO_SIZE];
pthread_mutex_t mutex;

int main(int argc, char *argv[])
{
     int x, sockfd, newsockfd, portno;
     socklen_t clilen;
     char buffer[256];
     struct sockaddr_in serv_addr, cli_addr;
     int i=0;
 
     void *th_fun(void *);
     pthread_mutex_init(&mutex, NULL);

     if (argc < 2) {
         fprintf(stderr,"ERROR, no port provided\n");
         exit(1);
     }
     
     sockfd = socket(AF_INET, SOCK_STREAM, 0);
     if (sockfd < 0) 
        error("ERROR opening socket");
     bzero((char *) &serv_addr, sizeof(serv_addr));
     portno = atoi(argv[1]);
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr = INADDR_ANY;
     serv_addr.sin_port = htons(portno);
     if (bind(sockfd, (struct sockaddr *) &serv_addr,
              sizeof(serv_addr)) < 0) 
              error("ERROR on binding");
   
    listen(sockfd,CONN_SIZE);
    clilen = sizeof(cli_addr);
   
    while(i < CONN_SIZE)
    {
     newsockfd = accept(sockfd,
                 (struct sockaddr *) &cli_addr,
                 &clilen);

     if (newsockfd < 0)
      error("ERROR on accept");
    
     pthread_t mythread;
   
     pthread_create(&mythread, NULL, th_fun, &newsockfd);
     i++;
   }     
      
     close(sockfd);
     pthread_exit(NULL);
     return 0;
}

       void *th_fun(void *arg)
       {
         int *fd = (int *)arg;
         int i=0, n, sockfd = *fd, tsockfd;
         char buffer[256]; 
         bzero(buffer,256); 
                 
              
         n = read(sockfd,buffer,255);                            //reading username             
         if (n < 0) error("ERROR reading to socket");        

         printf("Hi! %s\n",buffer);
         
         pthread_mutex_lock(&mutex);                            //acquiring a lock

         if(repo!=NULL)                                         //inserting username into repo database
         { 
           while((repo[i]!=NULL)&&(repo[i]!=buffer))
           {
                i++;
           }
           if(repo[i] == NULL)
           {
             repo[i] = buffer; 
           }
         }   
         else
         {
           repo[i] = buffer; 
         }  
         
         n = write(sockfd, buffer, 255);
         if((n == -1)||(n == 0))
         {
           bzero(buffer,256);
           n = read(sockfd,buffer,255);                            //reading destination username              
           if (n < 0) error("ERROR reading to socket");        
           printf("Connecting to %s\n", buffer);        
  
           if(repo[0]!=NULL)                                       //searching whether dest_username is already there in the repo database
           { 
             while((repo[i]!=NULL)&&(repo[i] != buffer))          
             {  
               i++;
             }
             if(repo[i] == buffer)
             {
               printf("%d, %s, %s\n", i, repo[i], buffer);
               tsockfd = i;
               printf("User available, Connecting !");
            
               n = read(sockfd,buffer,255);                        //reading the actual message         
  
               n = write(tsockfd, buffer, 255);                    //writing the message to the destination        
             
               n = write(sockfd,"I got your message",18);
               if (n < 0) error("ERROR writing to socket");
             }
             else
             {
               printf("user %s not available\n", buffer);
             }
           }
           else
           {
             printf("No user available\n");
           }
        } 
        
        pthread_mutex_unlock(&mutex);                           //releasing a lock               
        
        pthread_exit(NULL);
       
       }
