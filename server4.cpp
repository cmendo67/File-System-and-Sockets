#include "Directory.h"
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <fstream>
#include <strings.h>
#include <stdlib.h>
#include <string>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include <cctype>

using namespace std;

//socket for server
int server_socket;
//new connection will have a new socket
int client_socket;
//port number of server
int port_number;
//Maximum number of clients
int max_client = 5;
//receive message from client
char buf1[1024];
//send message back to client
char buf2[1024];
//byte_count is for receiving data
int byte_count;
//Id of pid
pid_t pid;
//buffer for ls command
char lsbuff[1024 + 1];

FILE *fpipe;
Directory d("main");


//This will handle connection for each client
void *server_thread(void *socket_desc){

    //new connection will have a new socket
    //Get the socket descriptor
    int newsock = *(int*) socket_desc;
    char *message;

    //send some messages to client
    // message = "Welcome, I am your connection handler\n";

    // write(newsock, message, strlen(message));

     //Read incoming data from the remote side using recv
     //recv-ssize_t recv(int s, void *buf, size_t len, int flags);
     //byte_count = Returns the number of bytes actually received or -1 on error
    //If the remote side has closed the connection, recv() will return 0.
    //receive a message from client
    while(byte_count = recv(newsock, buf1,sizeof(buf1),0) > 0){
        cout <<"recv() value from bytes of date in buf1 is  " << byte_count << endl;
        //If byte_count = 0, then the client disconnected
        //If the remote side has closed the connection, recv() will return 0
        if(byte_count == 0){
            printf("The client has disconnected\n");
            break;
        }
        //If byte_count == -1, display error
        if(byte_count == -1){
            printf("There was a connection issue, receiving failed\n");
            break;
        }

        cout << "Received: " << buf1 << endl;

        // memset(buf1,0,sizeof(buf1));
        
        pid = fork();
         //Fork error condition
        if(pid < 0){
            cout << "Error: PID error \n";
            //ptints out error message
            perror("exec");
        }else if(pid == 0){
            if (buf1[0] == 'F'){
                int n = d.fileList.size();
                string msg = "\nSize: ";
                msg += to_string(n);

                char message[msg.size() + 1];
                msg.copy(message, msg.size() + 1);
                message[msg.size()] = '\0';
                send(newsock, message, strlen(message), 0);
             } 
             else if(buf1[0] == 'C') {
                string text = "";
                int j = 0;
                for(int i = 2; i < strlen(buf1); i++) {
                    if(buf1[i] != '\0') {
                        text += buf1[i];
                    }
                }
                text.erase(std::remove_if(text.begin(), text.end(), ::isspace), text.end());
                string msg = "";
                myFile f(text, "");
                if(d.exists(f)) {
                    msg += "1 a file of this name already exists";
                }
                else if(!d.exists(f)) {
                    msg += "0 file successfully created";
                    d.fileList.push_back(f);
                }
                else {
                    msg+= "2 other failure occured";
                }
                
                char message[msg.size() + 1];
                msg.copy(message, msg.size() + 1);
                message[msg.size()] = '\0';
                send(newsock, message, strlen(message), 0);
             }
             else if(buf1[0] == 'D') {
                string text = "";
                int j = 0;
                for(int i = 2; i < strlen(buf1); i++) {
                    text += buf1[i];
                }
                text.erase(std::remove_if(text.begin(), text.end(), ::isspace), text.end());
                myFile f(text);
                string msg = "";
                if(d.exists(f)) {
                    d.deleteFile(f);
                    msg += "0 successfully deleted the file";
                }
                else if(!d.exists(f)) {
                    msg += "1 a file of this name did not exist";
                }
                else {
                    msg+= "2 other failure occured";
                }
                char message[msg.size() + 1];
                msg.copy(message, msg.size() + 1);
                message[msg.size()] = '\0';
                send(newsock, message, strlen(message), 0);
             }
             else if(buf1[0] == 'L' && buf1[2] == '0') {
                string msg = "";
                if(d.fileList.size() == 0) {
                    msg += "\nERROR: NO FILES IN DIRECTORY";
                }
                else {
                    msg += d.fileListing();
                }
                char message[msg.size() + 1];
                msg.copy(message, msg.size() + 1);
                message[msg.size()] = '\0';
                send(newsock, message, strlen(message), 0);
             }
             else if(buf1[0] == 'L' && buf1[2] == '1') {
                string msg = "";
                if(d.fileList.size() == 0) {
                    msg += "\nERROR: NO FILES IN DIRECTORY";
                }
                else {
                    msg += d.directoryListings();
                }
                char message[msg.size() + 1];
                msg.copy(message, msg.size() + 1);
                message[msg.size()] = '\0';
                send(newsock, message, strlen(message), 0);
             }
             else if(buf1[0] == 'R') {
                string text = "";
                int j = 0;
                for(int i = 2; i < strlen(buf1); i++) {
                    text += buf1[i];
                }
                text.erase(std::remove_if(text.begin(), text.end(), ::isspace), text.end());
                myFile f(text);
                string msg = "";
                if(d.exists(f)) {
                    msg += "\n0 ";
                    msg += to_string(d.fileSize(f));
                    msg += " File Data:";
                    msg += d.readData(f);
                }
                else if (!d.exists(f)){
                    msg += "\n1 ERROR: FILE DOES NOT EXIST";
                }
                else {
                    msg+= "\n2 OTHER FAILURE OCCURED";
                }
                char message[msg.size() + 1];
                msg.copy(message, msg.size() + 1);
                message[msg.size()] = '\0';
                send(newsock, message, strlen(message), 0);
             }
             else if(buf1[0] == 'W') {
                string text = "";
                int j = 0;
                for(int i = 2; i < strlen(buf1); i++) {
                    text += buf1[i];
                }
                string file = "";
                string data = "";
                bool go = true;
                for(int i = 0; i < text.length(); i++) {
                    if(text.at(i) == ' ') {
                        go = false;
                    }
                    if(go) {
                        file+= text.at(i);
                    }
                    else {
                        if(text.at(i) != ' ') {
                            data+= text.at(i);
                        }
                        else {
                            data += " ";
                        }
                    }
                }
                cout << "File given:" << file << endl;
                cout << "Data given:" << data << endl;
                string msg = "";
                myFile f(file, "");
                if(d.exists(f)) {
                    d.changeData(f, data);
                    msg += "\n0 succesfully written";
                }
                else if(!d.exists(f)) {
                    msg+= "\n1 ERROR: FILE DOES NOT EXIST";
                }
                else {
                    msg+= "\n2 OTHER FAILURE OCCURED";
                }
                char message[msg.size() + 1];
                msg.copy(message, msg.size() + 1);
                message[msg.size()] = '\0';
                send(newsock, message, strlen(message), 0);
             }
             else{
            
            int dummyvaribale = 0;
            //Reverse buffer
            for(int i = strlen(buf1) - 1; i >= 0; i--) {
                buf2[dummyvaribale] = buf1[i];
                dummyvaribale++;
             }
        
             //echo message back to client
             // write(newsock, buf2, strlen(buf2));
             send(newsock, buf2, strlen(buf2), 0);

             //Reset because if you dont, buf1 or buf2 gets concatenated when client sends a message and response from 
             //from server will get concatenated from previous messages
             memset(buf1, 0, sizeof(buf1));
             memset(buf2, 0, sizeof(buf2));

             }
        }  else {
       //will block parent process until any of its children has finished
        wait(NULL);
         }
         //flush stdout
        //ensure that whatever you just wrote to a file/the console is indeed
       //written out on disk/the console.
        fflush(stdout);
    }
    return 0;
}

int main(int argc, char* argv[]){

    //Port number of server
    port_number = atoi(argv[1]);
    //Space to hold the IPV4 string
    char ip4[INET_ADDRSTRLEN];

    //Socket address used for the server
    struct sockaddr_in server_address;
    struct sockaddr_in new_address;

    //Initialize server_address struct
    memset(&server_address,0,sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(port_number);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    inet_ntop(AF_INET,&(server_address.sin_addr),ip4,INET_ADDRSTRLEN);

    //Create a TCP socket, creation returns -1 on failure
    server_socket = socket(AF_INET,SOCK_STREAM,0);

    //Check for errors in listening socket
    if(server_socket < 0){
        printf("Could not create listening socket\n");
        return 1;
    }else{
        printf("Socket created..\n");
    }
    //Now we have to bind in order to listen for incoming connections
    //on the created server address
    //If there is an error, will return -1
    if((bind(server_socket,(struct sockaddr*)&server_address,sizeof(server_address)))< 0){
        cout << "Could not bind socket\n" << endl;
        return 1;
    }
    
    //listening for incoming connections
    if(listen(server_socket,max_client) < 0){
        cout << "Could not open socket for listening\n" << endl;
        return 1;
    }else if(listen(server_socket,max_client) == 0){
         cout << "Listening for incoming connections...\n";
         cout << "Listening on port " << ntohs(server_address.sin_port) << endl;
         cout << "Server IP address is " << ip4 << endl;
    }
    
    pthread_t thread[5];
    sockaddr_in sockaddr_client;
    socklen_t clientSize = sizeof(sockaddr_client);
    int l = 1;

    while(l){
        
        for(int i = 0; i < 5; i++){
        if(client_socket = accept(server_socket,(struct sockaddr*)&new_address, &clientSize)){

            if(client_socket == -1){
                cout << "Failed connection. A client failed to connect...\n";
            }                
                cout << "Connection successfull. Client has been accepted...\n";

                if(pthread_create(&thread[i], NULL,server_thread,&client_socket) < 0){
                    perror("Could not create thread");
                    return 1;
                }
          }
        }
        for(int j = 0; j < 5; j++){
            //Now join the thread , so that we dont terminate before the thread
		        pthread_join(thread[j] , NULL);
		        puts("Handler assigned");   
     
            if(client_socket < 0){
		      perror("accept failed");
		      return 1;
	        }
        }
    }
    return 0;
}