/* 
 * tcpclient.c - A simple TCP client
 * usage: tcpclient <host> <port>
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

/* 
 * error - wrapper for perror
 
 */
 
void error(char *msg) {
    perror(msg);
    exit(0);
}

int main(int argc, char **argv) {
    int sockfd, portno, n;
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
    if (connect(sockfd, &serveraddr, sizeof(serveraddr)) < 0) 
      error("ERROR connecting");

    /* get message line from the user */
    printf("Please enter filename : ");
    
    bzero(fname, BUFSIZE);
    gets(fname);   // reading file name
    
	 FILE *fp = fopen(fname,"rb");
	 int i;
    if(fp==NULL){
    	printf("File open error");
      return 1;   
    }  
    int x= fork(); 
    
    // MD5 checksum  computation for the file
    if( x == 0 ){
    	
    	int fd = open("md_5client.txt" ,  O_WRONLY | O_CREAT,0666);
    	close(1);
    	int newfd = dup(fd); //    redirecting output to   md_5client.txt
    	
    	execlp("md5sum" ,"md5sum" , "-t", (const char *)fname ,(char *)0);
    	exit(0);
    }
    else{
    	int status;
    	while(wait(&status) != x);
    }
    fseek(fp, 0L, SEEK_END);
    int filesize = ftell(fp);      // computing size of file
    rewind(fp);
    
    
    
    
    // send the filename to the server 
    n = write(sockfd, fname, BUFSIZE);
    if (n < 0) {
    	
      error("ERROR writing to socket");
    }
    
    // sending file size to the server
    n = write(sockfd , &filesize , sizeof(filesize));
    if (n < 0) {
    	
      error("ERROR writing to socket");
    }
    
    /* Read data from file and send it */
    while(1){
    	
     	unsigned char buff[BUFSIZE]={0};
      int nread = fread(buff,1,BUFSIZE,fp);
            //printf("Bytes read %d \n", nread);        

            /* If read was success, send data. */
      if(nread > 0){
      	write(sockfd, buff, nread);
      }
      if (nread < BUFSIZE ){
      	if (feof(fp)){
      		printf("End of file\n");
      		printf("File transfer completed for id: %d\n",sockfd);
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
    fread(md5checksum , 1 , BUFSIZE , fptr);
    rewind(fptr);
    fclose(fptr);
    remove("md_5client.txt");
    printf("client : %s\n",md5checksum);
    
    
    //  receiving MD5 checksum from the server
    bzero(buf, BUFSIZE);
    
    read(sockfd , buf , BUFSIZE);

    printf("server : %s\n",buf);
    if(strcmp(md5checksum , buf) == 0){          // checking match of checksums from server and client
    	printf("MD5 Matched !!! \n");
    }
    else{
    	printf("Oops !!! MD5 did not Match. \n");
    }
    
    close(sockfd);
    
    return 0;
}
