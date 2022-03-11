#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
} 

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

int main(int argc, char *argv[]){
  int connectionSocket, charsRead;
  char buffer[256];
  struct sockaddr_in serverAddress, clientAddress;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) { 
    fprintf(stderr,"USAGE: %s port\n", argv[0]); 
    exit(1);
  } 
  
  // Create the socket that will listen for connections
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0);
  if (listenSocket < 0) {
    error("ERROR opening socket");
  }

  // Set up the address struct for the server socket
  setupAddressStruct(&serverAddress, atoi(argv[1]));

  // Associate the socket to the port
  if (bind(listenSocket, 
          (struct sockaddr *)&serverAddress, 
          sizeof(serverAddress)) < 0){
    error("ERROR on binding");
  }

  // Start listening for connetions. Allow up to 5 connections to queue up
  listen(listenSocket, 5); 
  
  // Accept a connection, blocking if one is not available until one connects
  while(1){
    // Accept the connection request which creates a connection socket
    connectionSocket = accept(listenSocket, 
                (struct sockaddr *)&clientAddress, 
                &sizeOfClientInfo); 
    if (connectionSocket < 0){
      error("ERROR on accept");
    }

    // printf("SERVER: Connected to client running at host %d port %d\n", 
    //                       ntohs(clientAddress.sin_addr.s_addr),
    //                       ntohs(clientAddress.sin_port));

    // Get textfile from the client and display it
    memset(buffer, '\0', 256);
    // Read the client's message from the socket
    charsRead = recv(connectionSocket, buffer, 255, 0); 
    if (charsRead < 0){
      error("ERROR reading from socket");
    }
    // printf("SERVER: I received this from the client: \"%s\"\n", buffer);

    // Send a Success message back to the client (textfile)
    charsRead = send(connectionSocket, 
                    "I am the server, and I got your message", 39, 0); 
    if (charsRead < 0){
      error("ERROR writing to socket");
    }
    //store textfile name
    char *textfile = calloc(strlen(buffer) + 1, sizeof(char));
    strcpy(textfile, buffer);
    
    // Get keyfile from the client and display it
    memset(buffer, '\0', 256);
    // Read the client's message from the socket
    charsRead = recv(connectionSocket, buffer, 255, 0); 
    if (charsRead < 0){
      error("ERROR reading from socket");
    }
    // printf("SERVER: I received this from the client: \"%s\"\n", buffer);

    // Send a Success message back to the client (keyfile)
    charsRead = send(connectionSocket, 
                    "I am the server, and I got your message", 39, 0); 
    if (charsRead < 0){
      error("ERROR writing to socket");
    }
    
    //store keyfile name
    char *keyfile = calloc(strlen(buffer) + 1, sizeof(char));
    strcpy(keyfile, buffer);

    // printf("*textfile:%s\n", textfile);
    // printf("*keyfile:%s\n", keyfile);

    // Get keyfile from the client and display it
    memset(buffer, '\0', 256);
    // Read the client's message from the socket
    charsRead = recv(connectionSocket, buffer, 255, 0); 
    if (charsRead < 0){
      error("ERROR reading from socket");
    }
    // printf("SERVER: I received this from the client: \"%s\"\n", buffer);

    //Read in textfile
    long size;
    char* file_content;
    FILE *fp = fopen(textfile, "r");
    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    file_content = calloc(size, sizeof(char));
    fread(file_content, 1, size, fp);
    // printf("**file_content:\n%s\n", file_content);
    //strip newline
    file_content[size-1] = '\0';
    size -= 1; //adjust size for removal of newline character

    //Read in keyfile
    char* keyfile_content;
    fp = fopen(keyfile, "r");
    keyfile_content = calloc(size, sizeof(char));
    fread(keyfile_content, 1, size, fp);
    //strip newline
    keyfile_content[size] = '\0';

    //iterate through textfile and keyfile, encrpty file_contents
    int temp;
    int key;
    for(int i=0; i < size; i++){
      temp = file_content[i];
      if(temp == 32){
        temp = 26;
      }else{
        temp -= 65;
      }
      key = keyfile_content[i];
      if(key == 32){
        key = 26;
      }else{
        key -= 65;
      }
      temp += key;
      temp = temp % 27;
      file_content[i] = temp + 65;
    }


    //Send textfile size to client
    char str[10];
    sprintf(str, "%li", size);
    // printf("**longtostr:%s\n", str);
    charsRead = send(connectionSocket, str, strlen(str), 0); 
    if (charsRead < 0){
      error("ERROR writing to socket");
    }

    // Get textfile size confirm from the client and display it
    memset(buffer, '\0', 256);
    // Read the client's message from the socket
    charsRead = recv(connectionSocket, buffer, 255, 0); 
    if (charsRead < 0){
      error("ERROR reading from socket");
    }
    // printf("SERVER: I received this from the client: \"%s\"\n", buffer);

    // While loop to handle large files
    char send_buffer[1001];
    while(size > 0){
      if(strlen(file_content) > 1000){
        strncpy(send_buffer, file_content, 1000);
        send_buffer[1000] = '\0';
        file_content += 1000;
        size -= 1000;
      }else{
        strcpy(send_buffer, file_content);
        size -= strlen(file_content);
      }
      //Send textfile content to client
      // printf("**file_content:\n%s\n", file_content);
      charsRead = send(connectionSocket, send_buffer, strlen(send_buffer), 0); 
      if (charsRead < 0){
        error("ERROR writing to socket");
      }
      // printf("SERVER: size=%zi, file_content strlength =%zi\n", size, strlen(file_content));

      // Get textfile content confirm from the client and display it
      memset(buffer, '\0', 256);
      // Read the client's message from the socket
      charsRead = recv(connectionSocket, buffer, 255, 0); 
      if (charsRead < 0){
        error("ERROR reading from socket");
      }
      // printf("SERVER: I received this from the client: \"%s\"\n", buffer);
    }
    // Close the connection socket for this client
    close(connectionSocket); 
  }
  // Close the listening socket
  close(listenSocket); 
  return 0;
}
