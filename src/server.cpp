/*Server which handles login, exit, and certain command execution*/

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <utility>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <signal.h>

using namespace std;

#define BACKLOG 10	 // how many pending connections queue will hold

#define MAXLEN 100  // Maximum length of command accepted

/* Issues
 *
 * Need to move commons to sploit.h
 * Need to read from conf file
 * Need to write functions to  send and recv while handling errors
 * Shared variables between childs for w function
 * Prompt should be given by the server. ($, user $)
 * How to stop server
 * Handle: login $username dsjfl, pass $password ksdfj
*/


// Has the form User : (pass, login_status)
typedef unordered_map<string, pair<string, int> > UserMap;
// Has the form Command: command_id
typedef unordered_map<string, int > CmdMap;

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
	commands.insert(pair<string, int> ("logout", 1));
	commands.insert(pair<string, int> ("ls", 2));
	commands.insert(pair<string, int> ("ping", 3));
	commands.insert(pair<string, int> ("w", 4));
	commands.insert(pair<string, int> ("whoami", 5));
	commands.insert(pair<string, int> ("exit", 6));
	commands.insert(pair<string, int> ("mkdir", 9));

	// Port number
	PORT = 3490;
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
void client_login(const int new_fd, const string command, UserMap::iterator& info){

	/* params
	*
	* new_fd: Socket descriptor from client connection
	* command: string of format: login $USERNAME
	* info: UserMap iterator which will point to user's hash entry
	*/

	// For sername password which we receive
	string username;
	string password;

	// To save reply to send back
	char send_buf[MAXLEN];
	memset(send_buf, 0, MAXLEN);
	
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
		strcpy(send_buf, "Enter password, Format: pass $PASSWORD\n");
		send(new_fd, send_buf, strlen(send_buf), 0);
		
		// Receiving password
		numbytes = recv(new_fd, rec_buf, MAXLEN-1, 0);
		rec_buf[numbytes] = '\0';
		string temp(rec_buf);
		istringstream iss(temp);
		iss >> password;

		// Should be in format: pass $PASSWORD
		if (password != "pass"){
			strcpy(send_buf, "Wrong format\n$");
			send(new_fd, send_buf, strlen(send_buf), 0);
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
			send(new_fd, send_buf, strlen(send_buf), 0);
			return;
		}
		//Wrong password
		else{
			strcpy(send_buf, "Wrong password\n");
			send(new_fd, send_buf, strlen(send_buf), 0);
			return; 
		}
	}
	// Wrong username
	else{
		strcpy(send_buf,"Username not recognized\n");
		send(new_fd, send_buf, strlen(send_buf), 0);
		return; 
	}
}

/* 
* Function to parse and run command sent by client
* ret type: int
* ret 1: Closed
* ret 0: Normal exit
*/
int run_command(const int new_fd, string command, UserMap::iterator& info){

	/* params
	*
	* new_fd: client connection descriptor
	* command: command received by the client
	* info: Iterator to user's hash entry if he is logged in
	*/

	// To save reply to client
	char send_buf[MAXLEN];
	memset(send_buf, 0, MAXLEN);

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
						client_login(new_fd, command, info);
						// Returning because login function handles replies
						return 0;
					}
					break;

			// Logout
			case 1: if(info == users.end()){
						strcpy(send_buf, "You are not logged in!\n");
					}
					else{
						info->second.second = 0;
						info = users.end();
						strcpy(send_buf, "Logged out\n");
					}
					break;

			// ls
			case 2: if(info == users.end()){
						strcpy(send_buf, "You are not logged in!\n");
					}
					else{
						strcpy(send_buf, "ls output: \n");
					}
					break;
			// Ping
			case 3: strcpy(send_buf, "ping output: \n");
					break;
				
			// w
			case 4: if(info == users.end()){
						strcpy(send_buf, "You are not logged in!\n");
					}
					else{
						strcpy(send_buf, w_command());
					}
					break;

			// Whoami
			case 5: if (info != users.end()){
						string name = info->first + "\n";
						strcpy(send_buf, const_cast<char *> (name.c_str()));
					}
					else{
						strcpy(send_buf, "You are not logged in.\n");
					}
					break;
			// Exit
			case 6: if(info != users.end()){
						info->second.second = 0;
						info = users.end();
					}
					close(new_fd);
					// Returning as no need to send reply
					return 1;
			// mkdir
			case 9: strcpy(send_buf, "calling system functions: \n");
					break;
			
		}
	}

	// Sending back reply to command
	send(new_fd, send_buf, strlen(send_buf), 0);
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

	// Loop to handle all commands sent by one client
	while(1){

		// Buffer to recieve command
		char command[MAXLEN];
		int numbytes;
		memset(command, 0, MAXLEN);

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
		if ((run_command(new_fd, string(command), info)) == 1){
			// Returns only when connection is closed
			return NULL;
		}

	}

}

int main()
{

	// Fill global variables
	parse_conf_file();
	
	// Variables needed for socket
	int sockfd, new_fd;  
	struct sockaddr_in serv_addr, client_addr;
	socklen_t addr_len = sizeof(struct sockaddr_in);
	int yes=1;

	// Filling in server address, port and family
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_addr.s_addr = htons(INADDR_ANY);
	serv_addr.sin_port = htons(PORT);

	// Creating a TCP socket
	// Q: Move into function?
	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("server: socket");
		exit(1);
	}

	// To reuse socket
	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		perror("setsockopt");
		exit(1);
	}

	// Binding port
	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) == -1) {	
		close(sockfd);
		perror("server: bind");
		exit(1);
	}

	// Start listening
	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

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
		printf("server: got connection from %s\n", ip4);

		// Create thread to handle each client
		pthread_t thread;
		pthread_create(&thread, NULL, handle_client, &new_fd);
	}
	return 0;
}

