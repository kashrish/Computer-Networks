/* 
 * udpserver.c - A UDP echo server 
 * usage: udpserver <port_for_server>
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

typedef struct {
	int seq;               //  sequence no.
	int len;               //  len  = file size  or size of file chunk
	int chunks ;           //   # chunks the file is divided into
	char buf[BUFSIZE];     //   bytes of a file chunk  or filename
	
} packet ;

/*typedef struct {
	char filename[BUFSIZE] ;
	int filesize;
	int chunks;
	int seqno ;
} finfo ;*/

packet arr[5000];

/*
 * error - wrapper for perror
 */
void error(char *msg) {
  perror(msg);
  exit(1);
}
void find_md5_checksum(char *filename ){
	 int x= fork(); 
    
    // MD5 checksum  computation for the file
    if( x == 0 ){
    	
    	int fd = open("md_5server.txt" ,  O_WRONLY | O_CREAT,0666);  
    	close(1);
    	int newfd = dup(fd); //    redirecting output to   md_5client.txt
    	
    	execlp("md5sum" ,"md5sum" , "-t", (const char *)filename ,(char *)0);
    	exit(0);
    }
    else{
    	int status;
    	while(wait(&status) != x);
    }
	
}
int main(int argc, char **argv) {
  int sockfd; /* socket file descriptor - an ID to uniquely identify a socket by the application program */
  int portno; /* port to listen on */
  int clientlen; /* byte size of client's address */
  struct sockaddr_in serveraddr; /* server's addr */
  struct sockaddr_in clientaddr; /* client addr */
  struct hostent *hostp; /* client host info */
  char buf[BUFSIZE]; /* message buf */
  char filename[BUFSIZE];
  char *hostaddrp; /* dotted decimal host addr string */
  int optval; /* flag value for setsockopt */
  int n; /* message byte size */
  int seqno , ack , filesize , chunks ;
  // check command line arguments 
  if (argc != 2) {
    fprintf(stderr, "usage: %s <port_for_server>\n", argv[0]);
    exit(1);
  }
  portno = atoi(argv[1]);

  // socket: create the socket 
  sockfd = socket(AF_INET, SOCK_DGRAM, 0);
  if (sockfd < 0) 
    error("ERROR opening socket");

  /* setsockopt: Handy debugging trick that lets 
   * us rerun the server immediately after we kill it; 
   * otherwise we have to wait about 20 secs. 
   * Eliminates "ERROR on binding: Address already in use" error. 
   */
   
   
  optval = 1;
  setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int));
	     

  // build the server's Internet address
  bzero((char *) &serveraddr, sizeof(serveraddr));
  serveraddr.sin_family = AF_INET;
  serveraddr.sin_addr.s_addr = htonl(INADDR_ANY);
  serveraddr.sin_port = htons((unsigned short)portno);

  // bind: associate the parent socket with a port 
  if (bind(sockfd, (struct sockaddr *) &serveraddr, 
	   sizeof(serveraddr)) < 0) 
    error("ERROR on binding");
    //gethostbyaddr: determine who sent the datagram
  /* hostp = gethostbyaddr((const char *)&clientaddr.sin_addr.s_addr, 
			  sizeof(clientaddr.sin_addr.s_addr), AF_INET);
    if (hostp == NULL)
      error("ERROR on gethostbyaddr");
    hostaddrp = inet_ntoa(clientaddr.sin_addr);
    if (hostaddrp == NULL)
      error("ERROR on inet_ntoa\n");
    printf("server received datagram from %s (%s)\n", 
	   hostp->h_name, hostaddrp);
     printf("server received %d/%d bytes: %s\n", strlen(buf), n, buf); */  

  // main loop: wait for a datagram, then echo it
  //  sendto: echo the input back to the client 
   // recvfrom: receive a UDP datagram from a client
  clientlen = sizeof(clientaddr);
  while (1) {
  
  	 seqno = -1;		 
    packet obj;
    bzero(&obj ,sizeof(packet));
    //  receiving file information from client
    
    while(1){ 
    	bzero(&obj , sizeof(packet));
		n = recvfrom(sockfd, &obj , sizeof( packet ), 0, (struct sockaddr *) &clientaddr, &clientlen);
		if (n < 0) error("ERROR in recvfrom");
		ack = obj.seq;
      n = sendto(sockfd, &ack, sizeof(int), 0, (struct sockaddr *) &clientaddr, clientlen); // sending acknowledgement
		if (n < 0) error("ERROR in sendto"); 
      if(ack != seqno) continue;
      seqno++;
      break;
    }
    strcpy(filename , obj.buf);
    chunks = obj.chunks ;
    filesize = obj.len ;
    
    printf("%s %d %d\n",filename , filesize , chunks);
    
    //   now server will receive file
    
    FILE *fp ;
	 fp = fopen("ABCD" , "ab");
    int bsum = 0;
    printf("Receiving file...");
    while(filesize > bsum){
		 while(1){ 
		 	bzero(&obj , sizeof(packet));
			n = recvfrom(sockfd, &obj , sizeof( packet ), 0, (struct sockaddr *) &clientaddr, &clientlen);
			if (n < 0) error("ERROR in recvfrom");
			ack = obj.seq;
		   n = sendto(sockfd, &ack, sizeof(int), 0, (struct sockaddr *) &clientaddr, clientlen); // sending acknowledgement
			if (n < 0) error("ERROR in sendto"); 
		   if(ack != seqno) continue;
		   seqno++;
		   break;
		 }
		 //printf("%d %d\n" , obj.seq , obj.len);
		 fwrite(obj.buf, 1,obj.len,fp);
		 bsum = bsum + obj.len ;
    }
    fclose(fp);
    printf("\nFile OK....Completed\n"); 
    bzero(&obj , sizeof(packet));
	 find_md5_checksum("ABCD");	 
	 FILE *fptr = fopen("md_5server.txt" , "r");
	 fread(obj.buf , 1 , 32, fptr);                // retreiving MD5checksum from file
	 printf("server : %s\n",obj.buf);
	 rewind(fptr);
	 fclose(fptr);	 
	 remove("md_5server.txt");
	 n = sendto(sockfd,&obj , sizeof(obj), 0, (struct sockaddr *) &clientaddr, clientlen); // sending acknowledgement
    
  }
  return 0;
}
