/*Server which handles login, exit, and certain command execution*/

#include <iostream>
#include <string>
#include <sstream>
#include <unordered_map>
#include <utility>
#include <fstream>
#include "../include/sploit.h"
#include <algorithm>
#include <vector>
#include <iterator>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>
#include <sys/sendfile.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#include <sched.h>
#include<sys/types.h>
#include<sys/wait.h>
#include <signal.h>
#include <dirent.h>

using namespace std;

#define MAXLEN 1024 // Maximum length of command accepted
#define STACK_SIZE (1024 * 1024)    /* Stack size for cloned child */

#define CONFIG_FILE "sploit.conf"

/* Issues
 *
 * Need to read from conf file
 * How to stop server
 * Handle: login $username dsjfl, pass $password ksdfj
 * Need to remove common headers
 * ls overflow
 * making newfd malloc has some problems
 * 
*/


// Has the form User : (pass, login_status)
typedef unordered_map<string, pair<string, int> > UserMap;
// Has the form Command: command_id
typedef unordered_map<string, pair<string, string> > CmdMap;

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
char BASE[4097]; // Maximum allowed path in linux

/*Signal handler function to wait on child process*/
void sigchld_handler(int s)
{
	// waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;

	while(waitpid(-1, NULL, WNOHANG) > 0);

	errno = saved_errno;
}

/* Takes in char* command, executes it, and fills in send_buf with output */
void execution(const char *command, char* send_buf){

	
	FILE *pf = NULL;
	fprintf(stderr, "%s\n", command);
	// Setup our pipe for reading and execute our command.
	if((pf = popen(command,"r")) != NULL){
		char *ln = NULL;
		size_t len = 0;
		int numbytes = 0;
		int offset = 0;
		while ((numbytes=getline(&ln, &len, pf)) != -1){
			// BUG: Will overflow
			strncpy(send_buf+offset, ln, MAXLEN-1-offset);
			offset += (numbytes);
		}
		free(ln);
		pclose(pf);
	}
	else{
	    strcpy(send_buf, "ERROR: Could not run command\n");
	}
	return;
}

// Init for implemented commands
void init(){
	
	commands.insert(make_pair("login", make_pair("", "")));
	commands.insert(make_pair("ping",  make_pair("ping -c 1", "")));
	commands.insert(make_pair("ls",    make_pair("ls -l 2>&1", "")));
	commands.insert(make_pair("cd",    make_pair("", "")));
	commands.insert(make_pair("get",   make_pair("", "")));
	commands.insert(make_pair("put",   make_pair("", "")));
	commands.insert(make_pair("date",  make_pair("date 2>&1", "")));
	commands.insert(make_pair("whoami", make_pair("", "")));
	commands.insert(make_pair("w",     make_pair("", "")));
	commands.insert(make_pair("logout", make_pair("", "")));
	commands.insert(make_pair("exit",  make_pair("", "")));
}

/* Parses conf file and fills in global variables */
void parse_conf_file(){
	// Initialize users and commands for now

	// Users
	users.insert(make_pair("nir", make_pair("123", 0)));
	users.insert(make_pair("basa", make_pair("456", 0)));
	users.insert(make_pair("jojo", make_pair("789", 0)));

	// Commands parsed from aliases
	commands.insert(make_pair("echo", make_pair("echo", ""))); // Same number for all aliases

	// Port number
	PORT = 3490;

	// Will have to handle if config file has a long base
	strcpy(BASE, ".");
}

int parse_config() {
	ifstream configFile(CONFIG_FILE);
	if(configFile.is_open()) {
		string line;
		while(getline(configFile, line)) {
			line.erase(line.begin(), find_if(line.begin(), line.end(), [] (int ch) {return !std::isspace(ch);}));
			if(line[0] == '#' || line.empty()) {
				continue;
			}
			istringstream iss(line);
			vector<string> tokens(istream_iterator<string>{iss}, istream_iterator<string>());
			for(auto token = tokens.begin(); token != tokens.end(); token++) {
				if((*token).compare("base") == 0) {
					if(tokens.size() != 2) {
						return -1;
					}
					strncpy(BASE,(*++token).c_str(), 4096);
					break;
				} else if((*token).compare("user") == 0) {
					if(tokens.size() != 3) {
						return -1;
					}
					string user = *++token;
					string pass = *++token;
					users.insert(make_pair(user, make_pair(pass, 0)));
					break;
				} else if((*token).compare("port") == 0) {
					if(tokens.size() != 2) {
						return -1;
					}	
					PORT  = stoi(*++token);
					break;
				} else if((*token).compare("alias") == 0) {
					if(tokens.size() < 3) {
						return -1;
					}
					string name = *++token;
					string cmd = *++token;
					//string params = accumulate(++token, tokens.end(), string(""));
					string params;
					params.clear();
					for(auto i = ++token; i != tokens.end(); ++i) {
						params += *i;
						if(i != tokens.end() - 1) {
							params += " ";
						}
					}
					cout << "params is " << params << endl;
					if(tokens.size() == 3) {
						//params = "NULL";
					}
					commands.insert(make_pair(name, make_pair(cmd, params)));
					break;
				} else {
					// invalid command?
					break;
				}
			}

		}
	
		cout << "---------CONFIG READ--------------" << endl;
		cout << "base is " << BASE << endl;
		cout << "port is " << PORT << endl;
		cout << "Users are : " << endl;
		for(auto it = users.begin(); it != users.end(); it++) {
			cout << (*it).first << "\t" << (*it).second.first << endl;
		}
		
		cout << "Commands are: " << endl;		
		for(auto it = commands.begin(); it != commands.end(); it++) {
			cout << (*it).first << "\t";
			//for(auto jt = (*it).second.first.begin(); jt != (*it).second.first.end(); jt++) {
			//	cout << (*jt) << " ";
			//}
			cout << (*it).second.first << "\t";
			cout << (*it).second.second;
			cout << endl;
		}
		cout << "------------END-------------------" << endl;
	
		configFile.close();
	} else {
		return -1;
	}
	return 0;
}

					
					



/* Function which will create home directories for all users*/
int create_home_dirs(){

	if (chdir(BASE) == -1){
		perror("ERROR: Invalid base directory");
		return -1;
	}
	for(auto iter : users){
		char* name = const_cast<char*> (iter.first.c_str());
		DIR* dir = opendir(name);
		if (dir)
		{
			/* Directory exists. */
			fprintf(stderr, "Dir already exists.\n");
			closedir(dir);
		}
		else if (ENOENT == errno)
		{
			/* Directory does not exist. */
			if(mkdir(name, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1){
				perror("ERROR: Failed to create directory");
				return -1;
			}
		}
		else
		{
			perror("ERROR: Failed to create directory");
			return -1;
		}	
	}
	return 0;
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
			strcpy(send_buf, "ERROR: Failed to allocate port for transfer.\n");
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
			strcpy(send_buf, "ERROR: Failed to allocate port for transfer.\n");
		}
	}
	// Error accessing file
	else{
		strcpy(send_buf, "ERROR: Could not open file for transfer.\n");
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
		strcpy(send_buf, "ERROR: Failed to allocate port for transfer.\n");
		return;
	}

	if (receive_socket != -1){
		
		
		struct args* argument = (args*) malloc(sizeof(struct args));
		
		int file = open(filename.c_str(), O_CREAT | O_WRONLY, 0666 );
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
		strcpy(send_buf, "ERROR: Failed to allocate port for transfer.\n");
	}

}


/* Sends back a list of logged in users */
// BUG: Can overflow buffer if there are a large number of users (Can be left as is)
char * w_command(){

	string list = "";

	for(auto iter : users){

		if (iter.second.second != 0)
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
			strcpy(send_buf, "ERROR: Wrong format\n");
			return;
		}
		
		// Comparing password
		if(iss >> password && password == pass){
			// Pointing iterator to user's entry and changing login status
			info = users.find(username);

			info->second.second += 1;
			strcpy(send_buf, "Login successful\n");
			chdir(info->first.c_str());
			return;
		}
		//Wrong password
		else{
			strcpy(send_buf, "ERROR: Wrong password\n");
			return; 
		}
	}
	// Wrong username
	else{
		strcpy(send_buf,"ERROR: Username not recognized\n");
		return; 
	}
}

/* Fucntion to get name of current directory */
string get_cur_dir(){

	// Getting absolute path of current directory
	char* cur = getcwd(NULL, 0);
	
	char* temp = NULL;
	char* save_ptr = NULL;
	string curdir;

	// Iterate though the absolute path till you reach end dir
	temp = strtok_r(cur, "/\n", &save_ptr);
	curdir = string(temp);
	while((temp = strtok_r(NULL, "/\n", &save_ptr)) != NULL){
		curdir = string(temp);
	}
	free(cur);
	// Curdir has name of current directory
	return curdir;

}

/*Splits path sent by user and tries to chdir into each folder in path*/
char* step_chdir(char* path, string home, char* send_buf){

	// Saving current directory incase the path entered was invalid
	char* cwd = getcwd(NULL, 0);

	// We first assume that path is valid
	int invalid = 0;

	char* cur = NULL;
	char* save_ptr = NULL;
	cur = strtok_r(path, "/\n", &save_ptr);
	do{
		// If user is trying to cd into parent
		if ((strcmp("..", cur)) == 0){
			// To check if user is in his home directory
			if (home == get_cur_dir()){
				// user is not allowed to leave his home directory
				invalid = 1;
				break;
			}	
		}
		if(chdir(cur) == -1){
			invalid = 1;
			break;
		}

	}while ((cur = strtok_r(NULL, "/\n", &save_ptr)) != NULL);

	if(invalid == 1){
		chdir(cwd);
		free(cwd);
		strcpy(send_buf, "ERROR: Invalid path\n");
	}
	else{
		free(cwd);
		strcpy(send_buf, "Success\n");
	}
	
}

/* Function to parse and run command sent by client */
int run_command(const int new_fd, char* command_cstr, UserMap::iterator& info, char* send_buf){

	/* params
	*
	* new_fd: client connection descriptor
	* command_str: command received by the client in char pointer format
	* info: Iterator to user's hash entry if he is logged in
	* send_buf: Reply buffer which will be filled in with command output
	* 
	* ret
	* 
	* Normal Exit: 0
	* Closed: -1
	*/

	// Tokenize command received to get the first word
	string command(command_cstr);
	istringstream iss(command);
	string com;
	iss >> com;

	// If command is not recognized
	if(commands.find(com) == commands.end()){
		if (strcmp(command_cstr, "\n") == 0){
			return 0;
		}
		strcpy(send_buf, "ERROR: Unknown command\n");
	}
	// If command is in map
	else{
		// Find command in hashmap
		CmdMap::iterator cmd_iter = commands.find(com);


		///////////
		/* Login */
		///////////
		if (com == "login"){

			if (info != users.end()){
				strcpy(send_buf, "ERROR: Already logged in!\n");
			}
			else{
				client_login(new_fd, command, info, send_buf);
				// Returning because login function handles replies
				return 0;
			}
		}



		////////
		/* ls */
		////////
		else if (com == "ls"){

			if(info == users.end()){
				strcpy(send_buf, "ERROR: You are not logged in!\n");
			}
			else{
				// Execute command string in hash entry
				execution(cmd_iter->second.first.c_str(), send_buf);
			}
		}



		////////
		/* cd */
		////////
		else if (com == "cd"){

			if(info == users.end()){
				strcpy(send_buf, "ERROR: You are not logged in!\n");
			}
			else{
				string path;
				if(iss >> path){
					// Try to cd into specified path and fill send_buf with result of operation
					step_chdir(const_cast<char*>(path.c_str()), info->first, send_buf);
				}
				else{
					// If no path is entered
					strcpy(send_buf, "ERROR: Enter path\n");
				}	
			}
		}



		/////////
		/* get */
		/////////
		else if (com == "get"){

			if(info == users.end()){
				strcpy(send_buf, "ERROR: You are not logged in!\n");
			}
			else{
				string filename;
				if (iss >> filename){
					send_file(new_fd, filename, send_buf, info);
				}
				else{
					strcpy(send_buf, "ERROR: Please enter filename\n");
				}
			}
		}



		/////////
		/* put */
		/////////
		else if (com == "put"){
			if(info == users.end()){
				strcpy(send_buf, "ERROR: You are not logged in!\n");
			}
			else{
				string filename, size;
				if (iss >> filename && iss >> size){
					receive_file(new_fd, filename, size, send_buf, info);
				}
				else{
					strcpy(send_buf, "ERROR: Please enter a valid filename and size\n");
				}
			}
		}



		//////////
		/* date */
		//////////
		else if (com == "date"){
			if(info == users.end()){
				strcpy(send_buf, "ERROR: You are not logged in!\n");
			}
			// Execute command string in hash and fill send_buf with output
			else{
				execution(cmd_iter->second.first.c_str(), send_buf);
			}
		}
			


		////////////
		/* whoami */
		////////////
		else if (com == "whoami"){

			if (info != users.end()){
				string name = info->first + "\n";
				strcpy(send_buf, const_cast<char *> (name.c_str()));
			}
			else{
				strcpy(send_buf, "ERROR: You are not logged in.\n");
			}
		}



		///////
		/* w */
		///////
		else if (com == "w"){

			if(info == users.end()){
				strcpy(send_buf, "ERROR: You are not logged in!\n");
			}
			else{
				strcpy(send_buf, w_command());
			}
		}



		////////////
		/* Logout */
		////////////
		else if (com == "logout"){

			if(info == users.end()){
				strcpy(send_buf, "ERROR: You are not logged in!\n");
			}
			else{
				info->second.second -= 1;
				info = users.end();
				strcpy(send_buf, "Logged out\n");
			}
		}



		//////////
		/* Exit */
		//////////
		else if (com == "exit"){

			if(info != users.end()){
				info->second.second -= 1;
				info = users.end();
			}
			close(new_fd);
			// Returning as no need to send reply
			return -1;
		}



		//////////////////////////////////////////////////
		/* Alias command with parameter already present */
		//////////////////////////////////////////////////
		else if(cmd_iter->second.second != ""){

			string to_exec;
			to_exec = cmd_iter->second.first + " " + cmd_iter->second.second + " 2>&1";
			execution(to_exec.c_str(), send_buf);
		}



		////////////////////////////////////////////////
		/* Alias which expects user input as parameter*/
		////////////////////////////////////////////////
		// BUG: Has problems with "abc; ls" (Not Code injection though)
		else if (cmd_iter->second.second == ""){

			string to_exec;
			
			// Sanitize parameters i fany
			char *save_ptr;
			char *parameter = strtok_r(command_cstr, " \n", &save_ptr);
			parameter = strtok_r(NULL, ";\n", &save_ptr);

			// Building the command and executing
			if (parameter == NULL){
				to_exec = commands.find(com)->second.first + " 2>&1";
			}
			else{
				to_exec = commands.find(com)->second.first + " " + string(parameter) + " 2>&1";
			}
			
			execution(to_exec.c_str(), send_buf);

		}
		
		
	}

	// Success in parsing and executing command
	return 0;
}

/* Threads call this function to handle each client */
int handle_client(void* data){

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

	string hd;

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
			return 0;
		}
		command[numbytes] = '\0';

		// Try to run the command
		if ((run_command(new_fd, command, info, send_buf)) == -1){
			// Returns only when connection is closed
			exit(0);
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

	//init commands
	init();
	// Fill global variables after parsing conf file
	//parse_conf_file();

	int result = parse_config();
	if(result != 0) {
		fprintf(stderr, "ERROR: Error reading config file \n");
		exit(1);
	}
	// Create directory structure for all users
	if(create_home_dirs() == -1){
		fprintf(stderr, "ERROR: Error setting up home directories\n");
		exit(1);
	}

	// Signal handler to wait on children
	struct sigaction sa;
	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

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

		char *stack;                    /* Start of stack buffer */
		char *stackTop;                 /* End of stack buffer */
		stack = (char*) malloc(STACK_SIZE);
		if (stack == NULL){
			perror("malloc");
			continue;
		}
		stackTop = stack + STACK_SIZE;  /* Assume stack grows downward */

		// Create thread to handle each client
		int thread_id;
		thread_id = clone(handle_client, stackTop, SIGCHLD | CLONE_VM , &new_fd);
		if (thread_id == -1){
			perror("clone");
		}

	}
	return 0;
}

