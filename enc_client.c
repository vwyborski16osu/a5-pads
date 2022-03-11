#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

/**
* Client code
* 1. Create a socket and connect to the server specified in the command arugments.
* 2. Prompt the user for input and send that input as a message to the server.
* 3. Print the message received from the server and exit the program.
*/

// Error function used for reporting issues
void error(const char *msg) { 
  perror(msg); 
  exit(0); 
} 

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, 
                        int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address)); 

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname("localhost"); 
  if (hostInfo == NULL) { 
    fprintf(stderr, "CLIENT: ERROR, no such host\n"); 
    exit(0); 
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, 
        hostInfo->h_addr_list[0],
        hostInfo->h_length);
}

int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsWritten, charsRead;
  struct sockaddr_in serverAddress;
  char buffer[1001];
  // Check usage & args
  if (argc < 4) { 
    fprintf(stderr,"USAGE: %s plaintext key port\n", argv[0]); 
    exit(0); 
  } 

  // Check length of files
  long text_size;
  char* init_file_content;
  FILE *fp = fopen(argv[1], "r");
  fseek(fp, 0, SEEK_END);
  text_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);
  init_file_content = calloc(text_size, sizeof(char));
  fread(init_file_content, 1, text_size, fp);

  long key_size;
  fp = fopen(argv[2], "r");
  fseek(fp, 0, SEEK_END);
  key_size = ftell(fp);
  fseek(fp, 0, SEEK_SET);

  if(text_size > key_size){
    fprintf(stderr,"Error: key '%s' is too short\n", argv[2]); 
    exit(0);
  }else if(text_size < key_size){
    fprintf(stderr,"Error: key '%s' is too long\n", argv[2]); 
    exit(0);
  }

  int temp;
  for(int i=0; i < (text_size-2); i++){
    temp = init_file_content[i];
    if(temp != 32 && (temp < 65 || temp > 90)){
      fprintf(stderr,"enc_client error: input contains bad characters\n"); 
      exit(0);
    }
  }

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0); 
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, atoi(argv[3]));

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    error("CLIENT: ERROR connecting");
  }

  // Send textfile to server
  // Write to the server
  charsWritten = send(socketFD, argv[1], strlen(argv[1]), 0); 
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(argv[1])){
    // printf("CLIENT: WARNING: Not all data written to socket!\n");
  }

  // Get success message from server (textfile)
  // Clear out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  // printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
  
  // send keyfile to server
  charsWritten = send(socketFD, argv[2], strlen(argv[2]), 0); 
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(argv[2])){
    // printf("CLIENT: WARNING: Not all data written to socket!\n");
  }

  // Get return message from server (keyfile)
  // Clear out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  // printf("CLIENT: I received this from the server: \"%s\"\n", buffer);

  // send Waiting to server
  charsWritten = send(socketFD, "Waiting", 7, 0); 
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(argv[2])){
    // printf("CLIENT: WARNING: Not all data written to socket!\n");
  }

  // Get return message from server (textfile length)
  // Clear out the buffer again for reuse
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  int file_length;
  file_length = atoi(buffer);
  // printf("CLIENT: I received this from the server: \"%i\"\n", file_length);

  // send Waiting to server
  charsWritten = send(socketFD, "Waiting after file length", 7, 0); 
  if (charsWritten < 0){
    error("CLIENT: ERROR writing to socket");
  }
  if (charsWritten < strlen(argv[2])){
    // printf("CLIENT: WARNING: Not all data written to socket!\n");
  }
  
  char* file_content = calloc(file_length+1, sizeof(char));
  //while loop to handle large files
  int orig_file_length = file_length + 1;
  while(file_length > 0){
    // Get return message from server (textfile contents)
    // Clear out the buffer again for reuse
    memset(buffer, '\0', sizeof(buffer));
    // Read data from the socket, leaving \0 at end
    charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0); 
    if (charsRead < 0){
      error("CLIENT: ERROR reading from socket");
    }
    // printf("CLIENT: I received this from the server: \"%s\"\n", buffer);
    strcat(file_content, buffer);
    file_length -= strlen(buffer);
    // printf("CLIENT: Remaining file length %i\n", file_length);
    // send Waiting to server
    charsWritten = send(socketFD, "Waiting", 7, 0); 
    if (charsWritten < 0){
      error("CLIENT: ERROR writing to socket");
    }
  }
  file_content[orig_file_length] = '\0';
  file_content[orig_file_length-1] = '\n';
  printf("%s", file_content);
  // Close the socket
  close(socketFD); 
  return 0;
}