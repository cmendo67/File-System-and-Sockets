#include <stdio.h> 
#include <sys/socket.h> 
#include <stdlib.h> 
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h> 

using namespace std;
char read_data[1024];

//the ip address of the server
char* ip_address; 
//port number of server
int port_number;
//struct for server address
struct sockaddr_in server_address;
//server socket
int server_socket;
//message to server
char message[1024];
char server_reply[1024];
//prototype the function for reading socket responses
int response_server(int new_socket);

int main(int argc, char* argv[]) {
  
    //check the commandline args
    if (argc != 3) {
        perror("Error Invlaid Number of Command Line Arguments Provided.");
        printf("Enter ./client3.cpp <IP address of server> <port number of server>");
        return -1;
    }

    //get the ip from the arguments
    ip_address = argv[1];
    
    //get port number from arguments
    port_number = atoi(argv[2]);

    //check that the ip address is not null
    if (ip_address == NULL) {
        perror("Error Getting Ip Address");
        return -1;
    }

    // create socket
    server_socket = socket(AF_INET, SOCK_STREAM, 0);
	if (server_socket  < 0){ 
		printf("\n Could not create socket\n"); 
		return -1; 
	} 

	server_address.sin_family = AF_INET; 
	server_address.sin_port = htons(port_number);

    if (connect(server_socket, (struct sockaddr *)&server_address, sizeof(server_address)) < 0) 
	{ 
		printf("\nConnection Failed. Error\n"); 
		return -1; 
	} 

    int l = 1;
    //create a infinite loop to gather user input
    printf("\nClient: ");
    //keep communicating with server
	while(fgets(message, sizeof(message), stdin) != NULL)
	{
         //compare the string to exit to see if we should exit the client (case-insensitive)
        if (strncasecmp(message, "exit", 4) == 0) {
            printf("Exiting the Client\n");
            break;
        }

		//Send some data
		if(send(server_socket, message, sizeof(message), 0) < 0)
		{
			puts("Send failed");
			return 1;
		}
		
         //read the server response
        if ((response_server(server_socket)) == -1) {
            break;
        }

        printf("Response: %s\n", read_data);

         //print enter a command again
        printf("\nClient: ");

        //use memset to reset the buffer and cmd_str
        memset(message, 0, strlen(message));
        memset(read_data, 0, strlen(read_data));

	}
	
	close(server_socket);

    return 0;
}

//function to read the server response into the buffer
int response_server(int socket_fd) {
    //set an int variable to read from the socket
    int read_variable  = 0;

    //the read and count the number read
    read_variable = read(socket_fd, read_data, sizeof(message));

    if (read_variable == -1) {
        perror("Error Reading from client");
    }

    //return num_read
    return read_variable;
}