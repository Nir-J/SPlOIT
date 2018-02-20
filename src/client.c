/* Client which sends commands and just prints the reply from server */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>

#include <arpa/inet.h>

#define MAXLEN 100 // Max length of command which we send 

/* Issues
*
* Need to be C++
*/

void main(int argc, char *argv[])
{
	int sockfd, numbytes;  
	char buffer[MAXLEN];
	struct sockaddr_in serv_addr;
	int port = 0;

	if (argc != 3) {
	    fprintf(stderr,"usage: client server_ip server_port\n");
	    exit(1);
	}

	// If entered port is invalid
	if ((port = atoi(argv[2])) == 0 || port < 1024 || port > 65535){
		fprintf(stderr, "Please enter a valid port number");
		exit(1);
	}

	// Filling server address information
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(port);

	// Trying to assign IP
	if (inet_pton(AF_INET, argv[1], &serv_addr.sin_addr) != 1){
		fprintf(stderr, "Please enter a valid IP address");
		exit(1);
	}

	// Creating a TCP socket
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("Error while creating a socket");
		exit(1);
	}

	// Trying to connect to the server
	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
		perror("Error while connecting to the server");
		close(sockfd);
		exit(1);
	}

	printf("client: connecting to %s\n", argv[1]);

	// Main loop which takes user commands
	while(1){

		// Clearing out buffer
		memset(buffer, 0, MAXLEN);
		// Take user input
		// Q: Should prompt be sent from server sub?
		printf("$: ");
		if (fgets(buffer, MAXLEN-1, stdin) == NULL){
			fprintf(stderr, "Error reading command");
			close(sockfd);
			exit(1);
		}
		// Sending command to server
		if (send(sockfd, buffer, MAXLEN-1, 0) == -1){
			perror("send");
			continue;
		}
		// Receiving reply to command from server
		if ((numbytes = recv(sockfd, buffer, MAXLEN-1, 0)) == -1){
			perror("recv");
			continue;
		}
		// recv returns zero if server closes connection
		if (numbytes == 0){
			printf("Connection closed.\n");
			close(sockfd);
			exit(0);
		}
		buffer[numbytes] = '\0';
		
		printf("%s",buffer);
	}
}

