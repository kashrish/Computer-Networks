/* 
 * udpclient.c - A simple UDP client
 * usage: udpclient <host> <port>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

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

// error - wrapper for perror
void error(char *msg) {
    perror(msg);
    exit(0);
}
void find_md5_checksum(char *filename ){
	 int x= fork(); 
    
    // MD5 checksum  computation for the file
    if( x == 0 ){
    	
    	int fd = open("md_5client.txt" ,  O_WRONLY | O_CREAT,0666);  
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
	
    int sockfd, portno, n ,i , ack , seqno ,scnt , chunks;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];
    char fname[BUFSIZE];
	 unsigned char md5checksum[BUFSIZE] = {0};	
		    /* check command line arguments */
    if (argc != 3) {
       fprintf(stderr,"usage: %s <hostname> <port>\n", argv[0]);
       exit(0);
    }
    hostname = argv[1];
    portno = atoi(argv[2]);

    /* socket: create the socket */
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) 
        error("ERROR opening socket");
        
    //  timeout to implement stop and wait protocol
    
    struct timeval timeout ;      
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

	 if (setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,sizeof(timeout)) < 0)  // passing timeout to setsockopt
                
    		error("setsockopt failed\n");    

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

    serverlen = sizeof(serveraddr);
    
    
    /* get filename from the user */
    
    printf("Please enter filename : ");
    
    bzero(fname, BUFSIZE);
    gets(fname);   // reading file name
    
	 FILE *fp = fopen(fname,"rb");
	 
    if(fp==NULL){
    	printf("File open error");
      return 1;   
    }  
    fseek(fp, 0L, SEEK_END);
    int filesize = ftell(fp);      // computing size of file
    rewind(fp);
    
    find_md5_checksum(fname);
    
    packet fsobj , frobj;  //  fsobj packet will contain file info
    bzero(&fsobj , sizeof(packet) ) ;
    strcpy(fsobj.buf , fname);
    fsobj.len  = filesize ;              
    fsobj.chunks =  filesize/BUFSIZE ;   //   calculating no. of chunks of 1kb  BUFSIZE = 1024
    if(filesize % BUFSIZE != 0 ) fsobj.chunks++ ;
    chunks = fsobj.chunks ;
    seqno = -1 ;
    fsobj.seq = seqno ;
    while(1){
    	n = sendto(sockfd, &fsobj,sizeof(fsobj), 0, &serveraddr, serverlen); // sending file info to server
    	if(n < 0)  error("ERROR in sendto");
		n = recvfrom(sockfd, &ack, sizeof(int), 0, &serveraddr, &serverlen); // receiving filename ,filesize and no. of chunks from server
		if (n < 0) error("ERROR in recvfrom");
		if(n == 0) continue;                   // timeout case
		if(ack>=seqno){ seqno++ ; break ;}
    } 
    
    printf("%s %d %d\n",fsobj.buf , fsobj.len , fsobj.chunks);
    
    
    
    
    // dividing file into chunks of 1kb  and sending 
    
    
    scnt = 0 ;
    
    while(1){
    
     	bzero(&fsobj , sizeof(packet));
     	
      fsobj.seq = seqno;
      int nread = fread(fsobj.buf ,1,BUFSIZE,fp);
      fsobj.len = nread;
            // printf("Bytes read %d \n", nread);        

            // If read was success, send data. 
      if(nread > 0){
      	
      	
      	scnt++;
      	while(1){
      		n = sendto(sockfd, &fsobj,sizeof(fsobj), 0, &serveraddr, serverlen); 
      		if(n<=0 && scnt == chunks)  break; //  this implies last chunk is received on the client side
      		if(n < 0)  error("ERROR in sendto");
      		n = recvfrom(sockfd, &ack, sizeof(int), 0, &serveraddr, &serverlen); 
      		if (n < 0) error("ERROR in recvfrom");
      		if(n == 0) continue;                   // timeout case
				if(ack>=seqno){ seqno++ ; break ;}
      	}
      	
      	
      }
      if (nread < BUFSIZE ){
      	if (feof(fp)){
      		printf("End of file\n");
      	}
         if (ferror(fp)) {  
         	printf("Error reading\n");
         }
         break;
      }
    }
    fclose(fp);
    // computing MD5 checksum for this file
    
    FILE *fptr = fopen("md_5client.txt" , "r");
    fread(md5checksum , 1 , 32 , fptr);
    rewind(fptr);
    fclose(fptr);
    remove("md_5client.txt");
    printf("client : %s\n",md5checksum);
    
    
    
    
    
    
    
    bzero(&fsobj , sizeof(packet));
    
    n = recvfrom(sockfd, &fsobj, sizeof(packet), 0, &serveraddr, &serverlen); //  receiving md5 checksum from server
    
    if(strcmp(md5checksum , fsobj.buf) == 0){          // checking match of checksums from server and client
    	printf("MD5 Matched !!! \n");
    }
    else{
    	printf("Oops !!! MD5 did not Match. \n");
    } 
    
    
    return 0;
}
