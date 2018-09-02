#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
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
    FILE *fptr = fopen("md_5client.txt" , "r");
    fread(md5checksum , 1 , 32 , fptr);
    rewind(fptr);
    fclose(fptr);
    remove("md_5client.txt");
}

int main(int argc, char **argv) {
	
    int sockfd, portno, n ,i  , chunks;
    int serverlen;
    struct sockaddr_in serveraddr;
    struct hostent *server;
    char *hostname;
    char buf[BUFSIZE];
    char fname[BUFSIZE];
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
        
    //  timeout to implement GBN protocol
    
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
    	printf("File open error\n");
      return 1;   
    }  
    fseek(fp, 0L, SEEK_END);
    int filesize = ftell(fp);      // computing size of file
    rewind(fp);
    chunks = filesize / BUFSIZE ;
    if(filesize % BUFSIZE != 0) chunks++;
    printf("%d %d\n" , filesize , chunks);
    packet* pktlist = (packet *)malloc((chunks+2) * sizeof(packet));
    
    // pktlist[0] will contain file info
    bzero(&pktlist[0] , sizeof(packet));
    pktlist[0].seq = 0;
    strcpy(pktlist[0].buf , fname) ; // storing filename
    pktlist[0].len = filesize ;    // filesize
    pktlist[0].chunks = chunks ;   // no. of chunks of 1kb
    
    // dividing file into 1kb chunks and storing it into pktlist
    
    i=1;
    while(1){
    	bzero(&pktlist[i] , sizeof(packet));
    	int nread = fread(pktlist[i].buf ,1,BUFSIZE,fp);
    	pktlist[i].seq = i;
    	pktlist[i].len = nread ;
    	i++;
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
    
    // now we will send the packets to server
    
    int base , nextseqnum , N = 3 ,ack = -1 , prev_ack = -1;
    base = 0; nextseqnum = 0;
    
    while(base <= chunks){
    	while(nextseqnum < base+N && nextseqnum <= chunks){
    		n = sendto(sockfd, &pktlist[nextseqnum] ,sizeof(pktlist[nextseqnum]), 0, &serveraddr, serverlen); 
    		if(n < 0)  error("ERROR in sendto");
    		//printf("send seqno : %d\n" , nextseqnum);
    		if(base == nextseqnum){
    			// start timer
    		}
    		nextseqnum++;
    	}
    	n = recvfrom(sockfd, &ack, sizeof(int), 0, &serveraddr, &serverlen); 
    	
    	if(n <= 0){ //  case of timeout
    		//  retransmitting the packets from seqno. base to nextseqnum - 1
    		for(i=base ;i < nextseqnum ;i++){
    			n = sendto(sockfd, &pktlist[i] ,sizeof(pktlist[i]), 0, &serveraddr, serverlen); 
    			if(n < 0)  error("ERROR in sendto");
    		}
    		printf("retransmission %d to %d\n" , base , nextseqnum - 1);
    		if(N >= 2) N = N/2 ; //  case of packet loss 
    		 
    	}
    	else{
    		// ack is received 
    		if(N + ack - prev_ack  <= chunks) 
    			N = N + ack - prev_ack ; 
    		
    		/* increasing the packet size so that by the time full window is acknowledged ,
    		  window size gets doubled  */
    		
    		
    		/* if(ack == prev_ack) {
    			N = N / 2;              
    		}
    		else {
    			N = N + ack - prev_ack ; 
    		} */
    		
    		base = ack + 1;
    		prev_ack = ack ;
    		//printf("ack received : %d\n" , ack);
    		if(base == nextseqnum) {
    			// stop timer
    		}
    		else{
    			// start timer
    		}
    	}
    }
    
    find_md5_checksum(fname) ;
    printf("md5checksum = %s\n" ,md5checksum);
    bzero(buf ,BUFSIZE);
    n = recvfrom(sockfd, buf, BUFSIZE , 0, &serveraddr, &serverlen); // receiving md5checksum from server
    if(strcmp(md5checksum , buf) == 0){
    	printf("md5 checksum matched !!!\n");
    }
    else{
    	printf("md5 checksum did not match !!!\n");
    }
    
    return 0;
}    
