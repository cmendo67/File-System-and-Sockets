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

#include <iostream>
#include <string>
#include <cstdlib>

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

//disk part
FILE* diskFile;
int delay = 0;
int sectors;
int cylinders;
int blocks = 0;
char* fileName;
int fileSize;
char reader[130];
int numChars = 128 / (int) sizeof(char);
char* writeData = (char*) malloc(numChars * sizeof(char));

//method to create disk open the file for writing/reading
int createDisk(char* fname) {
    diskFile = fopen(fname, "r+b"); //open or create file for writing at end of file
    if(!diskFile) {
        return 0;
    }
    fileSize = sectors * cylinders * 128;
    blocks = cylinders * sectors;
    ftruncate(fileno(diskFile), fileSize); //truncate file to specific length;
    return 1;
}

//boolean method to return if block given is valid
bool isValid(int block) {
    if(block < blocks && block > 0) {
        return true;
    }
    return false;
}

//boolean method to return true if read is valid, if not return false
bool read(int cylinder, int sector, char buff[]) {
    int block = (cylinder - 1) * (sectors) + sector;    //calculation based on cylinder and sector
    cout << "Read Block: " << block << endl;
    if(!isValid(block)) {
        cout << "error block input is invalid";
        // abort();
        return false;
    }
    else {
        usleep(delay);
        fseek(diskFile, block * 128, SEEK_SET); //Sets the position indicator associated with the stream to a beginning of file
        cout << "Reading from block: " << block << endl;
        if(fread(buff, 128, 1, diskFile) == 1) {
            cout << "Read Successful" << endl;
        }
        else {
            cout << "Error in read disk" << endl;
            abort();
        }
    }
    return true;
}

//boolean method used to write data to specified block if successful it returns true if not false
bool write(int sector, int cylinder, char *data) {
    int block = (cylinder - 1) * (sectors) + sector;
    if(!isValid(block)) {
        cout << "error block input is invalid";
        // abort();
        return false;
    }
    else {
        usleep(delay);
        fseek(diskFile, block * 128, SEEK_SET);
        if(fwrite(data, 128, 1, diskFile) == 1) {
            cout << "Written successful " << endl;
        } 
        else {
            cout << "Error in write disk";
            abort();
        }
    }
    return true;
}
 

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
            if (buf1[0] == 'l' && buf1[1] == 's'){
            
                int chars_read; 

                memset(lsbuff, 0,sizeof(lsbuff));
            
                fpipe = popen("ls", "r");

                if(fpipe != NULL){
                    chars_read = fread(lsbuff,sizeof(char),sizeof(lsbuff),fpipe);
                    if(chars_read > 0){
                        cout << lsbuff << endl;
                        send(newsock,lsbuff,strlen(lsbuff),0);
                    }
                    pclose(fpipe);
                    exit(EXIT_SUCCESS);
                }
                 exit(EXIT_FAILURE);
             }
             else if(buf1[0] == 'I') {
                 cout << "Info Request Sent" << endl;
                 string msg = "\nCylinders: ";
                 msg += to_string(cylinders);
                 msg += "\nSectors: ";
                 msg += to_string(sectors);

                 char message[msg.size() + 1];
                 msg.copy(message, msg.size() + 1);
                 message[msg.size()] = '\0';
                 send(newsock, message, strlen(message), 0);
             }
             else if(buf1[0] == 'R') { //R reads the contents of the cylinder and sector given
                 int cylinder = (int)buf1[2] - 48;
                 int sector = (int)buf1[4] - 48;
                 string msg = "";
                 if(read(cylinder, sector, reader)) {
                     if(strlen(reader) == 0) {
                        send(newsock, "0", 1, 0);
                    }
                    else {
                        send(newsock, reader, sizeof(reader), 0);
                    }
                 }
                 else {
                    msg += "ERROR: READING FROM DISK";
                    char message[msg.size() + 1];
                    msg.copy(message, msg.size() + 1);
                    message[msg.size()] = '\0';
                    send(newsock, message, strlen(message), 0);
                 }
             }
             else if(buf1[0] == 'W') { //W writes to the cylinder and sector given with the data given
                int cylinder = (int)buf1[2] - 48;
                int sector = (int)buf1[4] - 48;
                string text = "";
                for(int i = 6; i < strlen(buf1); i++) {
                    text += buf1[i]; 
                }

                //seperating the byte size and data into different string below
                string byt = "";
                string data = "";
                bool go = true;
                for(int i = 0; i < text.length(); i++) {
                    if(text.at(i) == ' ') {
                        go = false;
                    }
                    if(go) {
                        byt+= text.at(i);
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
                //converting the string to int
                int bytes = std::atoi(byt.c_str());
                int j = 0;
                int a = 0;
                if(byt.length() == 1) {
                    a = 8;
                }
                else if(byt.length() == 2) {
                    a = 9;
                }
                else {
                    a = 10;
                }
                //filling the write data with the data given in the buffer
                for(int i = a; i < strlen(buf1); i++) {
                    writeData[j] = buf1[i];
                    j++;
                }
                string msg = "";
                cout << "Data given " << writeData << endl;
                if(bytes > 128) {
                    cout << "Number of bytes is to large" << endl;
                    msg += "Number of bytes is to large";
                    // abort();
                }
                if(bytes < 128) {
                    memset(writeData + bytes, '0', 128 - bytes);
                    if(write(cylinder, sector, writeData)) {
                        cout << "Data written to cylinder " << cylinder << " and sector " << sector << endl;
                        msg += "\nWrote to cylinder ";
                        msg += to_string(cylinder);;
                        msg += " and sector ";
                        msg += to_string(sector);
                    }
                    else {
                        msg+= "ERROR WRITING TO DISK";
                    }
                }


                char message[msg.size() + 1];
                msg.copy(message, msg.size() + 1);
                message[msg.size()] = '\0';
                send(newsock, message, strlen(message), 0);
                cout << "Sent data" << endl;
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

    // have to take in #delay, #cylinders, #sectors, filename
    delay = atoi(argv[2]);
    cylinders = atoi(argv[3]);
    sectors = atoi(argv[4]);
    fileName = argv[5];

    cout << fileName << endl;

    //createDisk(fileName);
    if(createDisk(fileName) == 0) {
        cout << "ERROR IN CREATING DISK ";
    }
    else {
        cout << "SUCCESS IN CREATING DISK";
    }



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
         cout << "Num of cylinders: " << cylinders << endl;
         cout << "Num of blocks: " << blocks << endl;
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
                cout << "Connection successfull. Client has been accepted...\n" << endl;

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