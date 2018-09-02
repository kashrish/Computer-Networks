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
#include <time.h>
#define BUFSIZE 1024
#define LOCALHOST 127.0.0.1

typedef struct {
	int seq;               //  sequence no.
	int len;               //  len  = file size  or size of file chunk
	int chunks ;           //   # chunks the file is divided into
	char buf[BUFSIZE];     //   bytes of a file chunk  or filename
	
} packet ;

unsigned char md5checksum[32] = {0};
double dprob = 0.4 ;               // drop probability


// error - wrapper for perror
void error(char *msg) {
    perror(msg);
    exit(0);
}
void find_md5_checksum(char *filename ){     // function to compute md5checksum
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
    FILE *fptr = fopen("md_5server.txt" , "r");
    bzero(md5checksum , 32);
    fread(md5checksum , 1 , 32 , fptr);
    rewind(fptr);
    fclose(fptr);
    remove("md_5server.txt");
}

int  drop_or_not(){
	int x = rand() % 10000 ;            //  to drop an acknowledgement or not
	double pr = x *1.0 / 10000 ;
	if(pr <= dprob) return 1;
	return 0 ;
}

int main(int argc, char **argv) {
	  srand(time(NULL));
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
		 
	  clientlen = sizeof(clientaddr);
	  //printf("enter drop probability : ");
	  //scanf("%f" , &dprob);
	  printf("server running ... \n");
	  while(1){
		int expseqnum = 0 , ack = -1 , filesize , chunks = 100000 ;
		packet rcv_pkt ;
		char filename[BUFSIZE] ;
		FILE *fp ;
		while( expseqnum <= chunks ){
			bzero(&rcv_pkt , sizeof(packet));
			n = recvfrom(sockfd, &rcv_pkt , sizeof( packet ), 0, (struct sockaddr *) &clientaddr, &clientlen);
			if(n < 0)  error("ERROR in recvfrom");
			if(rcv_pkt.seq == expseqnum){
				if(expseqnum == 0){
					// file info( filename , filesize and no. of chunks ) received here
					bzero(filename , BUFSIZE);
					strcpy(filename , rcv_pkt.buf) ;
					filesize = rcv_pkt.len ;
					chunks = rcv_pkt.chunks ;
	 				fp = fopen(filename, "ab");
	 				printf("filename = %s , no. of chunks = %d , filesize = %d \n",filename , chunks , filesize);
    				printf("Receiving file...\n");
				}
				else if(expseqnum >=1 && expseqnum <= chunks){
					fwrite(rcv_pkt.buf, 1,rcv_pkt.len,fp);  // appending file data 
					//printf("seqno = %d\n" , expseqnum);
					if(expseqnum == chunks) {
						printf("File received !!!\n");
					}
				}
				int flag = drop_or_not() ;
				if(flag == 0){
					n = sendto(sockfd, &expseqnum, sizeof(int), 0, (struct sockaddr *) &clientaddr, clientlen); // sending acknowledgement
				}
				ack = expseqnum ;
				expseqnum++;
			}
			else{
				int flag = drop_or_not() ;
				if(flag == 0){
					n = sendto(sockfd, &ack, sizeof(int), 0, (struct sockaddr *) &clientaddr, clientlen); // sending acknowledgement
				}
			}
		}	   
	  	fclose(fp);
	  	find_md5_checksum(filename);  // computing md5checksum
	  	printf("md5checksum = %s\n" , md5checksum);
	  	// sending md5checksum
	  	n = sendto(sockfd, md5checksum ,strlen(md5checksum), 0,(struct sockaddr *) &clientaddr, clientlen); 
	  }
	  
	  return 0;
  
}
