/* 
 * tcpserver.c - A simple TCP echo server 
 * usage: tcpserver <port>
 */
//#include <bits/stdc++.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#define max(x,y) x>y ? x:y
#define BUFSIZE 1024
#define MAX_CONN 10
#define PORT_NO 9002
#define TIMEOUT 1000
//using namespace std ;

int mfdset[MAX_CONN] ;

struct conninfo{
	int fd ;
	char ipaddress[BUFSIZE] ;
	int portno ;
	int timeout ;
};

struct conninfo curr_conn[MAX_CONN];          //  to keep record of current connections

struct friends{
	char name[BUFSIZE] ;
	char ipaddress[BUFSIZE] ;           //   info of friends
	int portno ;
};

struct friends ff[6] ;                        //   list of friends


void init_mfdset(){
	int i;
	for(i=0;i < MAX_CONN ;i++){
		mfdset[i] = -1;
		curr_conn[i].fd = -1;
		curr_conn[i].timeout = time(NULL);
	}
}

void addto_mfdset(int x_fd){
	int i;
	for(i=0;i < MAX_CONN ;i++){
		if(mfdset[i] != -1) continue ;
		mfdset[i] = x_fd;
		break;
	}
}

void removefrom_mfdset(int x_fd){
	int i;
	for(i=0;i < MAX_CONN ;i++){
		if(mfdset[i] != x_fd) continue ;
		mfdset[i] = -1;
		break;
	}
}

void init_readfds(fd_set *ptr){
	FD_ZERO(ptr);
	int i;
	for(i=0; i<MAX_CONN ;i++){
		if(mfdset[i] != -1) 
			FD_SET(mfdset[i] , ptr);
	}
}

int get_max_fd(){
	int i , mv = -1;
	for(i=0; i<MAX_CONN ;i++){
		mv = max(mv , mfdset[i]);
	}
	return mv ;
}

void error(char *msg) {
  perror(msg);
  exit(1);
}

void init_friends(){
	int i;
	
	strcpy(ff[1].name ,"Jack"); strcpy(ff[1].ipaddress , "127.0.0.1"); ff[1].portno = 9001 ;
	strcpy(ff[2].name ,"Tom"); strcpy(ff[2].ipaddress , "127.0.0.1"); ff[2].portno = 9002 ;
	strcpy(ff[3].name ,"Bob"); strcpy(ff[3].ipaddress , "127.0.0.1"); ff[3].portno = 9003 ;
	strcpy(ff[4].name ,"Harry"); strcpy(ff[4].ipaddress , "127.0.0.1"); ff[4].portno = 9004 ;
	strcpy(ff[5].name ,"Ron"); strcpy(ff[5].ipaddress , "127.0.0.1"); ff[5].portno = 9005 ;
	return ;
}
void* func(void* param){
	
	while(1){
		int i;
		for(i=0;i< MAX_CONN ;i++){
			if(curr_conn[i].fd == -1)  continue ;
			time_t tm = time(NULL) ;
			if(tm - curr_conn[i].timeout > TIMEOUT){
				close(curr_conn[i].fd);  // closing the connection from my side
		 		removefrom_mfdset(curr_conn[i].fd) ;   //  removing this fd  as we no longer need it
				curr_conn[i].fd = -1 ;
				bzero(curr_conn[i].ipaddress , BUFSIZE);
				curr_conn[i].portno = -1;
				curr_conn[i].timeout = time(NULL) ;
			}
		}
	}
	return NULL ;
}
int main(int argc, char **argv) {
	
  	
  
  
  
  int parentfd , childfd , portno , optval , n ,stdinp; 
  struct sockaddr_in serveraddr , clientaddr;  
  char buf[BUFSIZE]; 
  int clientlen ;
  fd_set readset ;
  struct timeval timeout; // parameter for select system call
  
  portno = PORT_NO ;
  
  timeout.tv_sec = 1000 ;
  timeout.tv_usec = 0 ;
	
  
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0) 
    error("ERROR opening socket");

  
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
	     

  
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);

  // bind: associate the parent socket with a port 
  if (bind(parentfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

   
   // listen: make this socket ready to accept connection requests 
   
  if (listen(parentfd,5) < 0) // allow 5 requests to queue up 
    error("ERROR on listen");
  printf("Chat Server Running ....\n");
  
   // main loop: wait for a connection request, echo input line, then close connection.
   // read: read input string from the client
   // write: echo the input string back to the client 
  clientlen = sizeof(clientaddr);
  
  stdinp = 0 ;    //   standard input file descripter
  
  init_mfdset();
  addto_mfdset(stdinp);  // adding std i/p 
  addto_mfdset(parentfd); // adding parent socket file descripter
  
  init_friends() ;
  
  pthread_t tid ;
  pthread_create(&tid , NULL , func , (void *)0) ;
  
  while (1) {
  		
	
    
    //printf("hello while\n");
    init_readfds(&readset);
    
    select(get_max_fd()+1 , &readset , NULL , NULL , NULL);
    
    if(FD_ISSET(parentfd , &readset)){
    	//printf("parentfd\n");
		 // accept: wait for a connection request 
		childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    	if (childfd < 0) 
      	error("ERROR on accept");
		bzero(buf, BUFSIZE);
    	int n = read(childfd ,buf , BUFSIZE);
  		
		addto_mfdset(childfd);   // adding the new connection fd
		//  now adding to curr_conn
		int i;
		for(i=0;i<MAX_CONN;i++){
			if(curr_conn[i].fd != -1) continue ;
			curr_conn[i].timeout = time(NULL) ;
			curr_conn[i].fd = childfd ;
			char fn[100];
			int j,c=0;
			bzero(fn , 100);
			for(j=0;buf[j] != ':';j++){
				if(buf[j] == ' ' ) continue ;
				fn[c++]= buf[j] ;
				
			}
			for(j=1;j<=5;j++){
				if(strcmp(fn , ff[j].name) == 0) {
					strcpy(curr_conn[i].ipaddress , ff[j].ipaddress);
					curr_conn[i].portno = ff[j].portno ;
					break;
				}
			}
			
			//printf("%s %d\n",curr_conn[i].ipaddress , curr_conn[i].portno );
			break;
		}
		
    	printf("%s\n" , buf);
    	
    }
    else if(FD_ISSET(stdinp, &readset)){
    	//printf("stdin\n");
    	//printf("hello\n");
    	bzero(buf, BUFSIZE);
    	read(stdinp , buf , BUFSIZE);  // reading from std i/p
    	// now we have to send data to the friend
    	char fn[100] , msg[BUFSIZE] ;
    	bzero(fn , 100) ; bzero(msg , BUFSIZE);
    	int i,c=0 ,j;
    	buf[strlen(buf)-1] = '\0' ;
    	for(i=0;buf[i] != '/';i++){
    		if(buf[i] != ' ')
    			fn[c++] = buf[i] ;	
    	}
    	strcpy(msg,ff[PORT_NO % 10].name);
    	msg[strlen(msg)] = ':';
    	strcpy(msg + strlen(msg), buf+i+1 );  // message to be sent
    	
    	// lets find the file descripter
    	int flag = 0,fridx;
    	for(i=1;i<=5;i++){
    		if(strcmp(fn , ff[i].name) == 0){
    			fridx = i;
    			break;
    		}
    	}
    	
    	if(i==6) continue ;
    	
    	for(j=0;j<MAX_CONN;j++){
    		
    		if(strcmp(ff[i].ipaddress , curr_conn[j].ipaddress) == 0 && ff[i].portno == curr_conn[j].portno){
    			//  still present in readset
    			//printf("upper\n");
    			int n = write(curr_conn[j].fd , msg , strlen(msg));
    			//printf("lower\n");
    			curr_conn[j].timeout = time(NULL);
    			if(n<=0){
    				// now we have to remove the old file discripter and make a new connection .
    				printf("write not working\n");
    				j = MAX_CONN ;
    			}
    			break;
    		}
    	}
    	if(j == MAX_CONN){
    		//  no connection with this friend so we have to create a connection
    		 //printf("hello\n");
			 int sockfd , portno, n;
			 struct sockaddr_in serveraddr;
			 struct hostent *server;
			 char *hostname;
			 

			 hostname = ff[fridx].ipaddress ;
			 portno = ff[fridx].portno;
			 
		    /* socket: create the socket */
			 sockfd = socket(AF_INET, SOCK_STREAM, 0);
			 if (sockfd < 0) 
				  error("ERROR opening socket");
				  
			 /* gethostbyname: get the server's DNS entry */
			 server = gethostbyname(hostname);
			 if (server == NULL) {
				  fprintf(stderr,"ERROR, no such host as %s\n", hostname);
				  exit(0);
			 }

			 /* build the server's Internet address */
			 bzero((char *) &serveraddr, sizeof(serveraddr));
			 serveraddr.sin_family = AF_INET;
			 bcopy((char *)server->h_addr, 
			  (char *)&serveraddr.sin_addr.s_addr, server->h_length);
			 serveraddr.sin_port = htons(portno);

			 /* connect: create a connection with the server */
			 if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) {
			 	printf("unable to connect with %s\n" , fn);
			 	continue ;
			 }
				//error("ERROR connecting");
			 n = write(sockfd , msg , strlen(msg));	
			 addto_mfdset(sockfd);   // adding the new connection fd
			 
			//  now adding to curr_conn
			int i;
			for(i=0;i<MAX_CONN;i++){
				if(curr_conn[i].fd != -1) continue ;
				curr_conn[i].fd = sockfd ;
				strcpy(curr_conn[i].ipaddress , ff[fridx].ipaddress ) ;
				curr_conn[i].portno =  ff[fridx].portno ;
				curr_conn[i].timeout = time(NULL);
				break;
			}	
			 	
    	}	
    	
    }
    else{
    	//  some old client is sending data 
    	int i;
    	for(i=0 ;i < MAX_CONN ;i++){
    		if(FD_ISSET(mfdset[i] , &readset)){
    			//  mfdset[i] has send data .
    			
    			int connfd = mfdset[i] ;
    			bzero(buf, BUFSIZE);
    			
    			int n = read(connfd ,buf , BUFSIZE);
    			if(n <= 0){
    				printf("\nconnection closed ...\n");
    				close(connfd);  // closing the connection from my side
    				removefrom_mfdset(connfd) ;   //  removing this fd  as we no longer need it
    				
    				//  now removing from  curr_conn
					int i;
					for(i=0;i<MAX_CONN;i++){
						if(curr_conn[i].fd != connfd) continue ;
						curr_conn[i].fd = -1 ;
						bzero(curr_conn[i].ipaddress , BUFSIZE);
						curr_conn[i].portno = -1;
						curr_conn[i].timeout = time(NULL) ;
						break;
					}
    				break;
    			}
    			else{
    				int i;
					for(i=0;i<MAX_CONN;i++){
						if(curr_conn[i].fd != connfd) continue ;
						curr_conn[i].timeout = time(NULL);
						break;
					}
    				printf("%s\n" , buf);
    			}
    		}
    	}
    	
    }
    
    //close(childfd);
  }
  pthread_join(tid , NULL);
}
/*
#include <stdio.h>
#include <time.h>
int main ()
{
    time_t sec;
    sec = time (NULL);

    printf ("Number of hours since January 1, 1970 is %ld \n", sec/3600);
    return 0;
}
*/
