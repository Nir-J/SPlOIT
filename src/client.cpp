/* Client which sends commands and just prints the reply from server */
#include<string>
#include<sstream>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/sendfile.h>

#include "../include/sploit.h"

#include <arpa/inet.h>

#define MAXLEN 1024 // Max length of command which we send 

using namespace std;
// Global variables
char server_ip[33];

/* Issues
 *
 * Need to delete comman headers
 * Need to automate
 */

struct args {
    int port;
    int file;
    long size;
};

void * receive_file(void * data) {

    struct args * argument = (struct args * ) data;

    // Connect to the server port
    int recv_socket;
    if ((recv_socket = connect_to_socket(argument->port, server_ip)) == -1) {
        fprintf(stderr, "%d and %s\n", argument->port, server_ip);
        fprintf(stderr, "Thread could not connect to server.");
        return NULL;
    }

    // Receiving file
    long remain_data = argument->size;
    int file = argument->file;
    char buffer[MAXLEN];
    int numbytes;

    while ((remain_data > 0) && ((numbytes = recv(recv_socket, buffer, MAXLEN - 1, 0)) > 0)) {
        if ((write(file, buffer, numbytes)) == -1) {
            perror("write");
        }
        remain_data -= numbytes;

    }
    close(file);
    close(recv_socket);
    return NULL; // are we handling this? 
}

void * send_file(void * data) {

    struct args * argument = (struct args * ) data;

    // Connect to the server port
    int send_socket;
    if ((send_socket = connect_to_socket(argument-> port, server_ip)) == -1) {
        fprintf(stderr, "%d and %s\n", argument-> port, server_ip);
        fprintf(stderr, "Thread could not connect to server.");
        return NULL;
    }

    // Assign fd and size
    int file = argument->file;
    long remain_data = argument->size;
    int sent_bytes;

    /* Sending file data */
    while ((remain_data > 0) && (sent_bytes = sendfile(send_socket, file, NULL, MAXLEN - 1))) {
        if (sent_bytes == -1) {
            perror("send");
        }
        remain_data -= sent_bytes;
    }
    close(file);
    close(send_socket);
    return NULL;
}

void handle_file_transfer(string filename, string reply, char * data) {

    pthread_t thread;
    istringstream iss((char * ) data);

    if (reply == "get") {

        // Get port information
        string portstring;
        iss >> portstring; // get
        iss >> portstring; // port
        iss >> portstring; // $PORT
        int port = atoi(portstring.c_str());

        // Getting file size
        string sizestring;
        iss >> sizestring; // size
        iss >> sizestring; // $SIZE
        // BUG: atoi
        long size = atoi(sizestring.c_str());

        // Create new file in write mode
        int file;
        if ((file = open(filename.c_str(), O_CREAT | O_WRONLY, 0666)) == -1) {
            perror("opening file");
            return;
        }

        // Alocating a struct to send arguments to thread
        struct args * argument = (struct args * ) malloc(sizeof(struct args));
        argument-> size = size;
        argument-> port = port;
        argument-> file = file;

        // Calling thread function
        pthread_create( & thread, NULL, receive_file, argument);
    } else {

        // Get port information
        string portstring;
        iss >> portstring; // put
        iss >> portstring; // port
        iss >> portstring; // $PORT
        int port = atoi(portstring.c_str());

        // Open file in read mode
        int file;
        if ((file = open(filename.c_str(), O_RDONLY)) == -1) {
            perror("opening file");
            return;
        }

        // Get file size
        struct stat file_stat;
        stat(filename.c_str(), & file_stat);

        // Alocating a struct to send arguments to thread
        struct args * argument = (struct args * ) malloc(sizeof(struct args));
        argument->size = file_stat.st_size;
        argument->port = port;
        argument->file = file;

        // Calling thread function
        pthread_create( & thread, NULL, send_file, argument);
    }
}


int defaultmode(char *argv[], int port)
{ 
    int sockfd, numbytes;  
    char buffer[MAXLEN];
    //int port = 0;


    // Connect to server
    if((sockfd = connect_to_socket(port, argv[1])) == -1){
        fprintf(stderr, "Error connecting.\n");
        exit(1);
    }
    // Saving server IP
    strcpy(server_ip, argv[1]);

    printf("Client: connecting to %s\n", argv[1]);

    // To save filename of get/put
    string token;

    // Main loop which takes user commands
    while(1){

        // Clearing out buffer
        memset(buffer, 0, MAXLEN);

        // Receiving reply to command / prompt from server
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

        // Check if reply was a get call
        istringstream iss(buffer);
        string reply;
        if (iss >> reply && (reply == "get" || reply == "put")){
            handle_file_transfer(token, reply, buffer);
        }

        // Take user input
        if (fgets(buffer, MAXLEN-1, stdin) == NULL){
            fprintf(stderr, "Error reading command");
            close(sockfd);
            exit(1);
        }
        buffer[strlen(buffer)] = '\0';

        // Saving filename in case of get / put
        istringstream tokenize(buffer);
        tokenize >> token;
        if (token == "get" || token == "put") {
            if (token == "put") {
                // If its put, we need to make sure client has the file
                struct stat file_stat;
                tokenize >> token;
                if (stat(token.c_str(), & file_stat) != 0) {
                    // Invalidate put command
                    strcpy(buffer, "put\n");
                }
            } else {
                tokenize >> token;
            }

        }

        // Sending command to server
        if (send(sockfd, buffer, strlen(buffer), 0) == -1){
            perror("send");
            continue;
        }   
        
        if (token == "exit"){
            close(sockfd);
            printf("Connection closed.\n");
            break;
        }
    }
    return 0;
}

//function that checks if a given filename exists or not


void automatedmode(char *argv[], int port, FILE *fout)
{   
    int sockfd, numbytes;  
    char buffer[MAXLEN];    
    FILE * fin;
    char * line = NULL;
    size_t len = 0;
    ssize_t read;

    if((fin = fopen(argv[3], "r"))== NULL){
        perror("error opening file");
        return;
    }

    //check from here 

    if((sockfd = connect_to_socket(port, argv[1])) == -1){
            fprintf(fout, "Error connecting.\n");
            return;
    }
    strcpy(server_ip, argv[1]);

    // Printing into out file
    fprintf(fout,"Client: connecting to %s\n", argv[1]);    
    
    // To save filename of get/put
    string token;

    // Input from in file
    while ((read = getline(&line, &len, fin)) != -1) {
        
        
        // Clearing out buffer
        memset(buffer, 0, MAXLEN);

        // Receiving reply to command / prompt from server
        if ((numbytes = recv(sockfd, buffer, MAXLEN-1, 0)) == -1){
            perror("recv");
            continue;
        }
        // recv returns zero if server closes connection
        if (numbytes == 0){
            fprintf(fout,"Connection closed.\n");
            close(sockfd);
            return;
        }
        buffer[numbytes] = '\0';

        // Write to out file
        fprintf(fout,"%s",buffer);

        // Check if reply was a get call
        istringstream iss(buffer);
        string reply;
        if (iss >> reply && (reply == "get" || reply == "put")){
            handle_file_transfer(token, reply, buffer);
        }

        // Getting command from in file
        strcpy(buffer,line);

        // Saving filename in case of get / put
        istringstream tokenize(buffer);
        tokenize >> token;
        if (token == "get" || token == "put") {
            if (token == "put") {
                // If its put, we need to make sure client has the file
                struct stat file_stat;
                tokenize >> token;
                if (stat(token.c_str(), & file_stat) != 0) {
                    // Invalidate put command
                    strcpy(buffer, "put\n");
                }
            } else {
                tokenize >> token;
            }

        }

        // Sending command to server
        if (send(sockfd, buffer, strlen(buffer), 0) == -1){
            perror("send");
            continue;
        }   
        
        if (token == "exit"){
            close(sockfd);
            fprintf(fout,"Connection closed.\n");
            return;
        }
        
    }
    // Sending exit to server
    if (send(sockfd, "exit\n", 5, 0) == -1){
        perror("send");
    }
    close(sockfd);
    fprintf(fout,"Connection closed.\n");
    return;
}

int main(int argc, char *argv[])
{
    
    int port = 0;

    // Command line arguments not provided
    //To accept default or automated mode
    if (argc!=3 && argc!=5) {
        fprintf(stderr,"usage: client server_ip server_port\n or \nusage: client server_ip server_port infile outfile\n");
        exit(1);
    }

    // If entered port is invalid
    if ((port = atoi(argv[2])) == 0 || port < 1024 || port > 65535){
        fprintf(stderr, "Please enter a valid port number\n");
        exit(1);
    }

    
    if(argc==3)
    {
        defaultmode(argv, port);
    }

    if(argc==5)
    {
        FILE* fout = NULL;
        fout = fopen(argv[4], "w");
        automatedmode(argv, port, fout);
        fclose(fout);
    }
    
    return 0;
}
