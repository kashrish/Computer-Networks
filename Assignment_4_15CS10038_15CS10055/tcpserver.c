/* 
 * tcpserver.c - A simple TCP echo server 
 * usage: tcpserver <port>
 */

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define BUFSIZE 1024

#if 0
/* 
 * Structs exported from in.h
 */

/* Internet address */
struct in_addr {
  unsigned int s_addr; 
};

/* Internet style socket address */
struct sockaddr_in  {
  unsigned short int sin_family; /* Address family */
  unsigned short int sin_port;   /* Port number */
  struct in_addr sin_addr;	 /* IP address */
  unsigned char sin_zero[...];   /* Pad to size of 'struct sockaddr' */
};

/*
 * Struct exported from netdb.h
 */

/* Domain name service (DNS) host entry */
struct hostent {
  char    *h_name;        /* official name of host */
  char    **h_aliases;    /* alias list */
  int     h_addrtype;     /* host address type */
  int     h_length;       /* length of address */
  char    **h_addr_list;  /* list of addresses */
}
#endif

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}

int main(int argc, char **argv) {
  int parentfd; /* parent socket */
  int childfd; /* child socket */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buffer */
  char filename[BUFSIZE];
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */

  /* 
   * check command line arguments 
   */
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  /* 
   * socket: create the parent socket 
   */
  parentfd = socket(AF_INET, SOCK_STREAM, 0);
  if (parentfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
  optval = 1;
  setsockopt(parentfd, SOL_SOCKET, SO_REUSEADDR, 
	     (const void *)&optval , sizeof(int));

  /*
   * build the server's Internet address
   */
  bzero((char *) &serveraddr, sizeof(serveraddr));

  /* this is an Internet address */
  serveraddr.sin_family = AF_INET;

  /* let the system figure out our IP address */
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);

  /* this is the port we will listen on */
  serveraddr.sin_port = htons((unsigned short)portno);

  /* 
   * bind: associate the parent socket with a port 
   */
  if (bind(parentfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");

  /* 
   * listen: make this socket ready to accept connection requests 
   */
  if (listen(parentfd, 5) < 0) /* allow 5 requests to queue up */ 
    error("ERROR on listen");
  printf("Server Running ....\n");
  /* 
   * main loop: wait for a connection request, echo input line, 
   * then close connection.
   */
  clientlen = sizeof(clientaddr);
  while (1) {

    /* 
     * accept: wait for a connection request 
     */
    childfd = accept(parentfd, (struct sockaddr *) &clientaddr, &clientlen);
    if (childfd < 0) 
      error("ERROR on accept");
    
    /* 
     * gethostbyaddr: determine who sent the message 
     */
     
     /*hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server established connection with %s (%s)\n", 
	   hostp->h_name, hostaddrp); */
	   
    
    /* 
     * read: read input string from the client
     
     */
    
    if( fork() == 0 ){
    
		 close(parentfd);
		  
		 bzero(filename, BUFSIZE);
		 n = read(childfd, filename, BUFSIZE); //   reading file name sent by client into filename 
		 
		 if (n < 0)  error("ERROR reading from socket");
		   
		 printf("file name :  %s\n", filename);
		 
		 int fsz;  // to store file size
		 n = read(childfd , &fsz , sizeof(int)); //  reading file size sent by client into fsz 
		 
		 if (n < 0)  error("ERROR reading from socket");
		   
		 printf("size of file = %d\n",fsz);   // fsz stores file size .
		 printf("Receiving file...");
		 FILE *fp ;
		 fp = fopen(filename , "ab");
		 
		 
		 bzero(buf, BUFSIZE);
		 
		 int bytesReceived = 0;
		 int sum =0 ;
		 // receiving file from the client 
		 while(fsz - sum > 0){                  //  checking whether remaning bytes left is > 0
		 	bytesReceived = read(childfd, buf , BUFSIZE) ;  // bytes read  into buf sent by client
		 	fwrite(buf, 1,bytesReceived,fp);
		 	sum = sum + bytesReceived ;                  // total  bytes transferred upto now .
		 }
		 
		 if(bytesReceived < 0)
		 {
		     printf("\n Read Error \n");
		 }
		 
		 
		 fclose(fp);
		 printf("\nFile OK....Completed\n");  
		 int x = fork();
		 
		 //  using execlp to execute md5sum command by forking a child process .
		 if(x== 0){
		 	
		 	int fd = open("md_5server.txt" ,  O_WRONLY | O_CREAT,0666);  // This file will store the md5 checksum
		 	close(1);
		 	int newfd = dup(fd); //    redirecting output to   md_5server.txt
		 	execlp("md5sum" ,"md5sum" , "-t", (const char *)filename ,(char *)0);
		 	exit(0);
		 }
		 else{
		 	int status;
		 	while(wait(&status) != x);  // waiting for child to terminate .
		 }
		 
		 
		 bzero(buf , BUFSIZE);
		 FILE *fptr = fopen("md_5server.txt" , "r");
		 fread(buf , 1 , BUFSIZE , fptr);                // retreiving MD5checksum from file
		 rewind(fptr);
		 fclose(fptr);
		 remove("md_5server.txt");
		 printf("server : %s\n",buf);
		 write(childfd , buf , BUFSIZE);                 //     sending MD5 checksum from server to client
    }
    close(childfd);
  }
}
