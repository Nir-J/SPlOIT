/*Server which handles login, exit, and certain command execution*/

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <utility>

#include "include/sploit.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/sendfile.h>


using namespace std;

#define MAXLEN 100  // Maximum length of command accepted

/* Issues
 *
 * Need to read from conf file
 * How to stop server
 * Handle: login $username dsjfl, pass $password ksdfj
*/


// Has the form User : (pass, login_status)
typedef unordered_map<string, pair<string, int> > UserMap;
// Has the form Command: command_id
typedef unordered_map<string, int > CmdMap;

struct args{
	int sockfd;
	int file;
	long size;
	int send;
};

// Global variables
UserMap users;
CmdMap commands;
int PORT;


/* Parses conf file and fills in global variables */
void parse_conf_file(){
	// Initialize users and commands for now

	// Users
	users.insert(make_pair("nir", make_pair("123", 0)));
	users.insert(make_pair("basa", make_pair("456", 0)));
	users.insert(make_pair("jojo", make_pair("789", 0)));

	// Commands can be enum too
	commands.insert(pair<string, int> ("login", 0));
	commands.insert(pair<string, int> ("ping" , 1));
	commands.insert(pair<string, int> ("ls"   , 2));
	commands.insert(pair<string, int> ("cd"   , 3));
	commands.insert(pair<string, int> ("get"  , 4));
	commands.insert(pair<string, int> ("put"  , 5));
	commands.insert(pair<string, int> ("date" , 6));
	commands.insert(pair<string, int> ("whoami", 7));
	commands.insert(pair<string, int> ("w"    , 8));
	commands.insert(pair<string, int> ("logout", 9));
	commands.insert(pair<string, int> ("exit"  , 10));
	commands.insert(pair<string, int> ("others", 11));


	// Port number
	PORT = 3490;
}

// Thread function which waits for client to connect to port
void* wait_for_connect(void* data){

	struct args* argument = (args*) data;

	// Variables needed for socket
	int new_fd; 
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);

	// Accepting connections
	new_fd = accept(argument->sockfd, (struct sockaddr *)&client_addr, &addr_len);
	if (new_fd == -1) {
		perror("accept");
		free(argument);
		return NULL;
	}

	// If send is true
	if (argument->send == 1){
		// Sending file
	    long remain_data = argument->size;
	    int sent_bytes;


	    /* Sending file data */
	    while ((remain_data > 0) && (sent_bytes = sendfile(new_fd, argument->file, NULL, MAXLEN-1)))
	    {
	    	remain_data -= sent_bytes;
	    }
	}
	// Receiving file
	else{

		int file = argument->file;
	    char buffer[MAXLEN];
	    int numbytes;
	    long remain_data = argument->size;

	    while ( (remain_data > 0) && ((numbytes = recv(new_fd, buffer, MAXLEN-1, 0)) > 0))
	    {
	            write(file, buffer, numbytes);
	            remain_data -= numbytes;
	    }
	}
	close(new_fd);
    close(argument->file);
    close(argument->sockfd);
	free(argument);
}


// Doesn't handle path for now
void send_file(int new_fd, string filename, char* send_buf, UserMap::iterator info){

	struct stat file_stat;   
  	if (stat (filename.c_str(), &file_stat) == 0){
  		// File exists

  		// Set up socket with any open port
  		int send_socket = listening_socket(0, 0);
  		struct sockaddr_in sin;
		socklen_t len = sizeof(sin);

		if (getsockname(send_socket, (struct sockaddr *)&sin, &len) == -1){
		    perror("getsockname");
		    strcpy(send_buf, "Failed to allocate port for transfer.\n");
		    return;
		}

  		if (send_socket != -1){
  			
  			int file = open(filename.c_str(), O_RDONLY);
  			
  			// Arguments to pass to thread which will handle transfer
  			// Do not pass pointers to local variables
  			struct args* argument = (args*) malloc(sizeof(struct args));
  			argument->file = file;
  			argument->sockfd = send_socket;
  			argument->size = file_stat.st_size;
  			argument->send = 1;

  			// Creating thread and sending it to accept connection
  			pthread_t thread;
  			pthread_create(&thread, NULL, wait_for_connect, argument);

  			// Send back port and file information to the client
  			sprintf(send_buf, "get port: %d size: %ld\n", ntohs(sin.sin_port), file_stat.st_size);
  		}
  		// Error in finding free socket
  		else{
  			strcpy(send_buf, "Failed to allocate port for transfer.\n");
  		}
  	}
  	// Error accessing file
  	else{
  		strcpy(send_buf, "Could not open file for transfer.\n");
  	}

}

// Doesn't handle path for now
void receive_file(int new_fd, string filename, string size_string, char* send_buf, UserMap::iterator info){

	// BUG: atoi
	long size = atoi(size_string.c_str());

	// Set up socket with any open port
	int receive_socket = listening_socket(0, 0);
	struct sockaddr_in sin;
	socklen_t len = sizeof(sin);

	if (getsockname(receive_socket, (struct sockaddr *)&sin, &len) == -1){
		perror("getsockname");
		strcpy(send_buf, "Failed to allocate port for transfer.\n");
		return;
	}

	if (receive_socket != -1){
		
		
		struct args* argument = (args*) malloc(sizeof(struct args));
		
		int file = open(filename.c_str(), O_WRONLY | O_CREAT);
		// No file descriptor
		argument->file = file;
		argument->sockfd = receive_socket;
		argument->size = size;
		argument->send = 0;

		// Creating thread and sending it to accept connection
		pthread_t thread;
		pthread_create(&thread, NULL, wait_for_connect, argument);

		// Send back port and file information to the client
		sprintf(send_buf, "put port: %d\n", ntohs(sin.sin_port));
	}
	// Error in finding free socket
	else{
		strcpy(send_buf, "Failed to allocate port for transfer.\n");
	}

}


/* Sends back a list of logged in users */
// BUG: Can overflow buffer if there are a large number of users (Can be left as is)
char * w_command(){

	string list = "";

	for(auto iter : users){

		if (iter.second.second == 1)
			list += (iter.first + " ");
	}
	list += "\n";

	// Have to cast string so that it can be sent over the buffer.
	return const_cast<char*> (list.c_str());

}

/* Handles everything related to loggin in an user */
void client_login(const int new_fd, const string command, UserMap::iterator& info, char* send_buf){

	/* params
	*
	* new_fd: Socket descriptor from client connection
	* command: string of format: login $USERNAME
	* info: UserMap iterator which will point to user's hash entry
	* send_buf: Reply buffer which will be filled in with command output
	*/

	// For sername password which we receive
	string username;
	string password;
	
	// To tokenize string
	istringstream iss(command);
	
	// Trying to get username
	iss >> username;

	// Checking username and pass
	if (iss >> username && (users.find(username) != users.end())){

		// Saving the actual password in pass
		string pass = users.find(username)->second.first;
		
		// Buffer to receive password
		char rec_buf[MAXLEN];
		memset(rec_buf, 0, MAXLEN);
		int numbytes = 0;
		
		// Asking for password
		strcpy(send_buf, "Enter password, Format: pass $PASSWORD\n$: ");
		send(new_fd, send_buf, strlen(send_buf), 0);
		
		// Receiving password
		numbytes = recv(new_fd, rec_buf, MAXLEN-1, 0);
		rec_buf[numbytes] = '\0';
		string temp(rec_buf);
		istringstream iss(temp);
		iss >> password;

		// Should be in format: pass $PASSWORD
		if (password != "pass"){
			strcpy(send_buf, "Wrong format\n");
			return;
		}
		
		// Comparing password
		if(iss >> password && password == pass){
			// Pointing iterator to user's entry and changing login status
			info = users.find(username);

			// If client is logged in using another system
			if (info->second.second == 1){
				strcpy(send_buf, "You are already logged in using another terminal.\n");
				info = users.end();
			}
			else{
				info->second.second = 1;
				strcpy(send_buf, "Login successful\n");
			}
			return;
		}
		//Wrong password
		else{
			strcpy(send_buf, "Wrong password\n");
			return; 
		}
	}
	// Wrong username
	else{
		strcpy(send_buf,"Username not recognized\n");
		return; 
	}
}

/* Function to parse and run command sent by client */
int run_command(const int new_fd, string command, UserMap::iterator& info, char* send_buf){

	/* params
	*
	* new_fd: client connection descriptor
	* command: command received by the client
	* info: Iterator to user's hash entry if he is logged in
	* send_buf: Reply buffer which will be filled in with command output
	* 
	* ret
	* 
	* Normal Exit: 0
	* Closed: -1
	*/

	// Tokenize command received
	istringstream iss(command);
	string com;
	iss >> com;

	// If command is not recognized
	if(commands.find(com) == commands.end()){
		strcpy(send_buf, "Unknown command\n");
	}
	// If command is in map
	else{

		switch(commands.find(com)->second){

			// Login
			case 0: if (info != users.end()){
						strcpy(send_buf, "Already logged in!\n");
					}
					else{
						client_login(new_fd, command, info, send_buf);
						// Returning because login function handles replies
						return 0;
					}
					break;

			// Ping
			case 1: strcpy(send_buf, "ping output: \n");
					break;

			// ls
			case 2: if(info == users.end()){
						strcpy(send_buf, "You are not logged in!\n");
					}
					else{
						strcpy(send_buf, "ls output: \n");
					}
					break;

			// cd
			case 3: break;

			//get
			case 4: if(0 && info == users.end()){
						strcpy(send_buf, "You are not logged in!\n");
					}
					else{
						string filename;
						if (iss >> filename){
							send_file(new_fd, filename, send_buf, info);
						}
						else{
							strcpy(send_buf, "Please enter filename\n");
						}
					}
					break;

			//put
			case 5: if(0 && info == users.end()){
						strcpy(send_buf, "You are not logged in!\n");
					}
					else{
						string filename, size;
						if (iss >> filename && iss >> size){
							receive_file(new_fd, filename, size, send_buf, info);
						}
						else{
							strcpy(send_buf, "Please enter filename and size\n");
						}
					}
					break;

			// date
			case 6: break;

			// Whoami
			case 7: if (info != users.end()){
						string name = info->first + "\n";
						strcpy(send_buf, const_cast<char *> (name.c_str()));
					}
					else{
						strcpy(send_buf, "You are not logged in.\n");
					}
					break;

			// w
			case 8: if(info == users.end()){
						strcpy(send_buf, "You are not logged in!\n");
					}
					else{
						strcpy(send_buf, w_command());
					}
					break;

			// Logout
			case 9: if(info == users.end()){
						strcpy(send_buf, "You are not logged in!\n");
					}
					else{
						info->second.second = 0;
						info = users.end();
						strcpy(send_buf, "Logged out\n");
					}
					break;

			// Exit
			case 10: if(info != users.end()){
						info->second.second = 0;
						info = users.end();
					}
					close(new_fd);
					// Returning as no need to send reply
					return -1;

			// others
			case 11: strcpy(send_buf, "calling system functions: \n");
					break;
		
		}
	}

	// Success in parsing and executing command
	return 0;
}

/* Threads call this function to handle each client */
void* handle_client(void* data){

	/* params
	*
	* data: Void pointer to int connection descriptor
	*/

	// Nobody logged in at first
	UserMap::iterator info = users.end();
	
	// Converting void ptr argument to int
	int new_fd = *((int*)data);

	// Send prompt to client
	send(new_fd, "$: ", 3, 0);

	// Loop to handle all commands sent by one client
	while(1){

		// Buffer to recieve command
		char command[MAXLEN];
		char send_buf[MAXLEN];
		int numbytes;
		memset(command, 0, MAXLEN);
		memset(send_buf, 0, MAXLEN);

		//Receive command from the client
		if ((numbytes = recv(new_fd, command, MAXLEN-1, 0)) == -1){
			perror("recv");
			close(new_fd);
			pthread_exit((void*)1);
		}
		// If client closes connection
		if (numbytes == 0){
			close(new_fd);
			return NULL;
		}
		command[numbytes] = '\0';

		// Try to run the command
		if ((run_command(new_fd, string(command), info, send_buf)) == -1){
			// Returns only when connection is closed
			return NULL;
		}

		// Adding prompt to reply
		strcpy(send_buf+strlen(send_buf), "$: ");

		// Sending reply to client
		if (send(new_fd, send_buf, strlen(send_buf), 0) == -1){
			perror("send");
			continue;
		}

	}

}

int main()
{

	// Fill global variables
	parse_conf_file();
	
	// Variables needed for socket
	int sockfd, new_fd; 
	struct sockaddr_in client_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	
	// Start listening on mentioned port
	sockfd = listening_socket(PORT, 1);

	// Ready
	printf("Server: waiting for connections...\n");

	// To server multiple clients
	// BUG: Turning off client ?
	while(1) {  
		
		// Accepting connections
		new_fd = accept(sockfd, (struct sockaddr *)&client_addr, &addr_len);
		if (new_fd == -1) {
			perror("accept");
			continue;
		}

		// Printing client IP address
		char ip4[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &client_addr.sin_addr, ip4, INET_ADDRSTRLEN);
		printf("Server: got connection from %s\n", ip4);

		// Create thread to handle each client
		pthread_t thread;
		pthread_create(&thread, NULL, handle_client, &new_fd);
	}
	return 0;
}

