target : server1 server2 server3 server4 server5 

server1 : tcpserver1.c
	gcc -w tcpserver1.c -o server1 -lpthread
	
server2 : tcpserver2.c
	gcc -w tcpserver2.c -o server2 -lpthread
	
server3 : tcpserver3.c
	gcc -w tcpserver3.c -o server3 -lpthread
	
server4 : tcpserver4.c
	gcc -w tcpserver4.c -o server4 -lpthread

server5 : tcpserver5.c
	gcc -w tcpserver5.c -o server5 -lpthread		

clean :
	rm server1 server2 server3 server4 server5 		
