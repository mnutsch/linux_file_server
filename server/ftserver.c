/****************************************************************************
* Name: ftserver.c
* Author: Matt Nutsch
* Date: 11-24-2017
* Description:
* This is a TCP file server for Linux. 
* Based in part on the server example at: http://www.binarytides.com/server-client-example-c-sockets-linux/
****************************************************************************/

#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h> //strlen, malloc
#include <sys/socket.h> //socket
#include <arpa/inet.h> //inet_addr
#include <unistd.h> //write
#include <pthread.h> //for threading , link with 
#include <dirent.h> //to read the directory
#include <signal.h> //for handling signals

#include <fcntl.h>

char directoryContents[1024]; //string to store the directory contents. assumes maximum characters of 1024
char readBuffer[262144]; //string to store the file contents. assumes maximum characters of 262144

socklen_t len; //used in reading the IP address of the connecting host
struct sockaddr_storage addr; //used in reading the IP address of the connecting host
char ipstr[INET6_ADDRSTRLEN]; //used in reading the IP address of the connecting host
int port; //used in reading the IP address of the connecting host
	
int debugging = 0; //set this to 1 to see detailed verbose output from the program
	
/********************************************************
 * int readDirectory()
 * Date: 11/25/2017
 * Author: Matt Nutsch
 * Description: Read the directory contents 
 * Based in part on the example at: https://faq.cprogramming.com/cgi-bin/smartfaq.cgi?answer=1046380353&id=1044780608
 ********************************************************/
int readDirectory()
{
  DIR *d;
  struct dirent *dir;
  int numberOfFiles = 0;
  
  d = opendir(".");
  if (d)
  {
    while ((dir = readdir(d)) != NULL)
    { 
	  if(numberOfFiles == 0)
	  {
		sprintf (directoryContents, "%s", dir->d_name);
	  }
	  else
	  {
		sprintf (directoryContents, "%s\n%s", directoryContents, dir->d_name);
	  }
	  
	  numberOfFiles = numberOfFiles + 1;
    }

    closedir(d);
  }

  return(0);
}

/********************************************************
 * int readFile()
 * Date: 11/25/2017
 * Author: Matt Nutsch
 * Description: Reads a file into the variable readBuffer.
 * Based in part on lecture content from the OSU class CS344.
 ********************************************************/
int readFile(char * argFileName)
{
	char* filePathFull = "./test.txt";
	int file_descriptor = 0;
	ssize_t nread;
	
	//reset the file read buffer
	memset( readBuffer, '\0', sizeof(char)*sizeof(readBuffer) );
	
	//create the filePathFull
	asprintf(&filePathFull, "./%s", argFileName);
		
	//open the file stream
	file_descriptor = open(filePathFull, O_RDONLY);
		
	//output an error message if there is a problem
	if(file_descriptor == -1)
	{
		fprintf(stderr, "Could not open %s\n", filePathFull);
		perror("Error in readFile()");
		exit(0);
	}
	
	memset(readBuffer, '\0', sizeof(readBuffer)); //clear the array before using it
	lseek(file_descriptor, 0, SEEK_SET); //reset the file pointer to the beginning of the file
	
	nread = read(file_descriptor, readBuffer, sizeof(readBuffer)); //read the file into the buffer
	
	return 1;
}

/********************************************************
 * int sendDataToPeer(char * destinationHostAddress, int destinationHostPort, char * directoryContentsString)
 * Date: 11/26/2017
 * Author: Matt Nutsch
 * Description: Send the directory contents to the port described
 * Based in part on the example at: http://www.csl.mtu.edu/cs4411.ck/www/NOTES/process/fork/exec.html
 * Based in part on content from the OSU class CS344.
 * Based in part on the example at: http://www.binarytides.com/server-client-example-c-sockets-linux/
 ********************************************************/
int sendDataToPeer(char * destinationHostAddress, int destinationHostPort, char * directoryContentsString)
{
	int sock;
	struct sockaddr_in server;
	char messageFormatted[1088];
	char server_reply[1024];
		
	int continueConnection = 1;
		
	//variables for the fgets buffer
	char *buffer;
	size_t bufsize = 32;
	size_t characters;
	size_t ln; //used for getting the length of the buffer contents
		
    pid_t pid;
    int status = 0;
	
    if ((pid = fork()) < 0) 
	{     
		/* fork a child process */
        printf("*** ERROR: forking child process failed\n");
        return(0);
    }
    else if (pid == 0) 
	{          
		/* the child process: */
	
		//allocate memory to the getline buffer
		buffer = (char *)malloc(bufsize * sizeof(char));
		if( buffer == NULL)
		{
			perror("Unable to allocate buffer");
			exit(1);
		}
		
		//Create socket
		sock = socket(AF_INET , SOCK_STREAM , 0);
		if (sock == -1)
		{
			printf("Could not create socket!\n");
		}
		
		if(debugging == 1)
			printf("Socket created\n");
		
		server.sin_addr.s_addr = inet_addr(destinationHostAddress);
		server.sin_family = AF_INET;
		server.sin_port = htons( destinationHostPort );
		
		//Connect to remote server
		if (connect(sock , (struct sockaddr *)&server , sizeof(server)) < 0)
		{
			perror("Error: Connection to the server failed.");
			return 0;
		}
		
		if( send(sock , directoryContentsString , strlen(directoryContentsString) , 0) < 0)
		{
			puts("Send failed");
			return 0;
		}
		
		if(debugging == 1)
			printf("Waiting for the peer...\n");
		
		//Receive a reply from the server
		if( recv(sock , server_reply , 2000 , 0) < 0)
		{
			puts("Receive failed");
			//break;
		}
				
		printf("%s\n",server_reply);
				
		close(sock);
		
    }
    else 
	{                              
		/* the parent: */
		
		if(debugging == 1)
			printf("sending data...\n");
		
        while (wait(&status) != pid)
		{
			/* wait for completion  */
			
			
        }
    }
	
	return 1;
}

/********************************************************
 * void *connection_handler(void *socket_desc)
 * Date: 11/25/2017
 * Author: Matt Nutsch
 * Description: This will handle connection for each client
 * Based in part on the example at: http://www.binarytides.com/server-client-example-c-sockets-linux/
 ********************************************************/
void *connection_handler(void *socket_desc)
{
    //Get the socket descriptor
    int sock = *(int*)socket_desc;
    int read_size;
    char *message , client_message[2000];
	
	char firstWord[64]; //string to hold the first word of the client message
	char secondWord[64]; //string to hold the second word of the client message
	char thirdWord[64]; //string to hold the second word of the client message
	
	int spaceFound = 0; //flag to tell us when to stop copying characters
	int secondSpaceFound = 0; //flag to tell us when to stop copying characters
	int i = 0; //iterator for loops
	int secondWordCharCount = 0; //used in processing the user input
	int thirdWordCharCount = 0; //used in processing the user input
	
	int ret = 0; //used in comparing strings
	
	int childExitMethod = -5; //used in checking status of last process
	pid_t childPID = -5;; //used in checking status of last process
	pid_t spawnPID; //used in forking new commands
	
	//ignore SIGPIPE
	signal(SIGPIPE, SIG_IGN);
	
    //Send a welcome messages to the client
	message = "Success! You are connected to the file server\n";
	write(sock , message , strlen(message));
	 
    //Receive a message from client
    while( (read_size = recv(sock , client_message , 2000 , 0)) > 0 )
    {
		
		//parse the message
		//memset(firstWord, '\0', sizeof(char)*strlen(firstWord)); //reset the first word by setting the first char to a null terminator. Otherwise chars overlap from previous input.				
		spaceFound = 0;
		secondSpaceFound = 0;
		secondWordCharCount = 0;
		thirdWordCharCount = 0;
		
		//iterate through the client message to get the first word
		
		if(debugging == 1)
			printf("The strlen of client message is %d\n", strlen(client_message));
		
		if(debugging == 1)
			printf("Analyzing the client message...\n");
		for(i = 0; i < (strlen(client_message)); i = i + 1) //iterate through all 
		{
			if(debugging == 1)
				printf("%c", client_message[i]);
			
			//if we find a space, then set the spaceFound flag.
			if((client_message[i] == ' ') && (spaceFound == 0))
			{
				spaceFound = 1;
				
				if(debugging == 1)
					printf(" - space found\n");
			}
			else if(spaceFound == 0)  //copy characters as long as a space was not found yet
			{
				firstWord[i] = client_message[i];
				if(debugging == 1)
					printf(" - adding to the first word\n");
			}
			else //if spaceFound == 1 but not this time
			{
				//if we find a second space, then set the secondSpaceFound flag.
				if(client_message[i] == ' ')
				{
					secondSpaceFound = 1;
					
					if(debugging == 1)
						printf(" - second space found\n");
				}
				else if(secondSpaceFound == 0) 
				{
					secondWord[secondWordCharCount] = client_message[i];
					secondWordCharCount = secondWordCharCount + 1;
					if(debugging == 1)
						printf(" - adding to the second word\n");
				}
				else
				{
					thirdWord[thirdWordCharCount] = client_message[i];
					thirdWordCharCount = thirdWordCharCount + 1;
					if(debugging == 1)
						printf(" - adding to the third word\n");
				}
			}
			
		}
		
		if(debugging == 1)
		{
			//output the first word for debugging purposes
			printf("\nThe first word of the client message was: %s\n", firstWord);
		
			//output the second word for debugging purposes
			printf("The second word of the client message was: %s\n", secondWord);
			
			//output the third word for debugging purposes
			printf("The third word of the client message was: %s\n", thirdWord);
		}
		
		ret = 0;
		ret = strcmp(firstWord, "-l");
		if(ret == 0)
		{
			printf("List directory requested on port %d\n", atoi(secondWord));

			//the user sent the command -l
			if(debugging == 1)
				printf("The user sent the command -l\n");
		
			//read the directory contents into a string
			readDirectory();
			if(debugging == 1)
				printf("The directory contents are:\n%s\n", directoryContents);
			
			//call the peer with the string
			sleep(1); //pause, so that the peer can start listening
			if(debugging == 1)
			{
				printf("Sending data to the peer...\n");
				printf("The peer IP address we are using is %s\n", ipstr);
				printf("The peer port we are using is %d\n", atoi(secondWord));
			}
			printf("Sending directory contents to %s:%d\n", ipstr, atoi(secondWord));
			sendDataToPeer(ipstr, atoi(secondWord), directoryContents); //send the data
			
		}
		else
		{
			ret = strcmp(firstWord, "-g");
			if(ret == 0)
			{
				printf("File '%s' requested on port %d.\n", secondWord, atoi(secondWord));
				
				//the user sent the command -g
				if(debugging == 1)
					printf("The user sent the command -g\n");
				
				//read the file contents into a string
				readFile(&secondWord[0]);
				if(debugging == 1)
					printf("The contents of file %s are:\n%s\n", secondWord, readBuffer);
				
				//call the peer with the string
				sleep(1); //pause, so that the peer can start listening
				if(debugging == 1)
				{
					printf("Sending data to the peer...\n");
					printf("The peer IP address we are using is %s", ipstr);
					printf("The peer port we are using is %d", atoi(thirdWord));
				}
				printf("Sending '%s' to %s:%d.\n", secondWord, ipstr, atoi(secondWord));
				sendDataToPeer(ipstr, atoi(thirdWord), readBuffer); //send the data
			}
		}
		
        //Send the message back to client
        write(sock , client_message , strlen(client_message));
    }
     
    if(read_size == 0)
    {
        if(debugging == 1)
			printf("Client disconnected\n");
        fflush(stdout);
    }
    else if(read_size == -1)
    {
        perror("recv failed");
    }
         
    //Free the socket pointer
    free(socket_desc);
     
    return 0;
}

/********************************************************
 * int main(int arg1, char *argv[])
 * Date: 11/25/2017
 * Author: Matt Nutsch
 * Description: This is the main body of code.
 * 
 ********************************************************/
int main(int arg1, char *argv[])
{
    int socket_desc , client_sock , c , *new_sock;
    struct sockaddr_in server , client;
    
	if(arg1 == 1)
	{
		printf("No port number was received. Rerun the program with the port number as an argument. \ni.e. ftserver 1234\n");
	}
	else
	{
		if(atoi(argv[1]) == 0)
		{
			printf("The 'port number' received was not a number. Rerun the program with the port number as an argument. \ni.e. ftserver 1234\n");
		}
		else
		{
			if(debugging == 1)
				printf("The arguments received were %d\n", atoi(argv[1]));
			
			//Create socket
			socket_desc = socket(AF_INET , SOCK_STREAM , 0);
			if (socket_desc == -1)
			{
				printf("Could not create socket");
			}
			
			if(debugging == 1)
				puts("Socket created");
			
			//Prepare the sockaddr_in structure
			server.sin_family = AF_INET;
			server.sin_addr.s_addr = INADDR_ANY;
			server.sin_port = htons( atoi(argv[1]) );
			
			//Bind
			if( bind(socket_desc,(struct sockaddr *)&server , sizeof(server)) < 0)
			{
				//print the error message
				perror("bind failed. Error");
				return 1;
			}
			
			if(debugging == 1)
				puts("bind done");
			
			//Listen
			listen(socket_desc , 3);
			
			//Accept and incoming connection
			printf("Server open on %d\n", atoi(argv[1]));
			
			c = sizeof(struct sockaddr_in);
			while( (client_sock = accept(socket_desc, (struct sockaddr *)&client, (socklen_t*)&c)) )
			{
				if(debugging == 1)
					printf("Connection accepted\n");
				
				pthread_t sniffer_thread;
				new_sock = malloc(1);
				*new_sock = client_sock;
				
				if( pthread_create( &sniffer_thread , NULL ,  connection_handler , (void*) new_sock) < 0)
				{
					perror("could not create thread");
					return 1;
				}
				
				//get the name of the host
				//based on "getpeername()" example at: https://beej.us/guide/bgnet/output/html/singlepage/bgnet.html#gethostnameman
				len = sizeof addr;
				getpeername(client_sock, (struct sockaddr*)&addr, &len);
				// deal with both IPv4 and IPv6:
				if (addr.ss_family == AF_INET) {
					struct sockaddr_in *s = (struct sockaddr_in *)&addr;
					port = ntohs(s->sin_port);
					inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
				} else { // AF_INET6
					struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
					port = ntohs(s->sin6_port);
					inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
				}
				
				printf("Connection from %s.\n", ipstr);
				
				//Now join the thread , so that we dont terminate before the thread
				//pthread_join( sniffer_thread , NULL);
				puts("Handler assigned");
			}
			
			if (client_sock < 0)
			{
				perror("accept failed");
				return 1;
			}
		}
	}
	
   
	
    return 0;
}

