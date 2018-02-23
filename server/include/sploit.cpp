#include "sploit.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>


#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>

/* Connects to the specified IP and port */
int connect_to_socket(int port, char * ip){

	/* params
	*
	* port: Port number to connect to
	* ip: String representation of IP to connect to
	*
	* ret
	* Success: Socket descriptor
	* Error: -1
	*/

	int sockfd;  
	struct sockaddr_in serv_addr;

	// Filling server address information
	memset(&serv_addr, 0, sizeof(serv_addr)); 
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Trying to assign IP
	if (inet_pton(AF_INET, ip, &serv_addr.sin_addr) != 1){
		fprintf(stderr, "Please enter a valid IP address\n");
		return -1;
	}

	// Creating a TCP socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error while creating a socket\n");
		return -1;
	}

	// Trying to connect to the server
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
		perror("Error while connecting to the server\n");
		close(sockfd);
		return -1;
	}
	return sockfd;
}


/* Function to create a listening connection and return socket descriptor */
int listening_socket(int port, int reuse){

	/* params
	*
	* port: Port on which socket will listen
	* reuse: Specifies if the port can be reused
	* 
	* ret
	* Success: Socket descriptor
	* Error: -1
	*/

	int sockfd;
	struct sockaddr_in serv_addr;

	// Filling in server address, port and family
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);
	serv_addr.sin_addr.s_addr = htons(INADDR_ANY);

	// Creating a TCP socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("server: socket");
		return -1;
	}

	// To reuse socket
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(int)) == -1) {
		perror("setsockopt");
		return -1;
	}

	// Binding port
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {	
		close(sockfd);
		perror("server: bind");
		return -1;
	}

	// Start listening
	if (listen(sockfd, 10) == -1) {
		perror("listen");
		return -1;
	}
	return sockfd;

}