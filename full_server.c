#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include<sys/socket.h> 
#include<arpa/inet.h> //get inet_addr from here 
#include<sys/stat.h>    
#include<sys/types.h>    
#include<unistd.h>  
#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <time.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/sendfile.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#define CHUNK 1024 /* read 1024 bytes at a time */
#define LISTENQ  1024  /* second argument to listen() */
#define MAXLINE 1024   /* max length of a line */
#define RIO_BUFSIZE 1024

#define BUF_SIZE 3000		/* Size of shared buffer */

int buffer[BUF_SIZE];  	/* shared buffer */
int add = 0;  			/* place to add next element */
int rem = 0;  			/* place to remove next element */
int num = 0;  			/* number elements in buffer */

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;  	/* mutex lock for buffer */
pthread_cond_t c_cons = PTHREAD_COND_INITIALIZER; /* worker_thread waits on this cond var */
pthread_cond_t c_prod = PTHREAD_COND_INITIALIZER; /* boss_thread waits on this cond var */

//void *boss_thread (void *param);
void *worker_thread (void *param);



//_______________________________________________________________________________________________________________

char* verify_request(char *data){

	char* file;
	char line[10000], method[10000], path[10000], protocol[10000], idx[20000], location[20000];
	char* error_msg;
	
	// Empty string handling
	if (strlen(data) <= 0)
		error_msg = "Client request not found ";
	
	// Check if the request consists of next line only
	if (strcmp(data, "\n" ) == 0 )
	    error_msg = "Bad Request : Request consists of next line only";

	// Check if the request consists of next line only
	if (strcmp(data, "\r\n" ) == 0 )
	    error_msg = "Bad Request : Request consists of next line and carriage return only";
	
	// Verify if all three components of the request are given
	if(sscanf(data, "%[^ ] %[^ ] %[^ ]", method, path, protocol) != 3 )
	    error_msg = "Bad Request : Method, path or protocol missing ";
	
	// Check if protocol is GET
	if (strcasecmp(method, "get" ) != 0 )
	    error_msg = "UnSupported Protocol";
	
	// Verify if filepath is correct
	if (path[0] != '/')
	    error_msg = "Bad Request :  File path not correct";
	
	// Verify if file path not correct (copied from micro_httpd.c)
	file = &(path[1]); // Get the second part of request 
	int length = strlen( file );
	if (file[0] == '/' || strcmp( file, ".." ) == 0 || strncmp( file, "../", 3 ) == 0 || strstr( file, "/../" ) != (char*) 0 || strcmp( &(file[length-3]), "/.." ) == 0)
	   error_msg = "Bad Request : Incorrect file descriptor";
	
	// Verify that the file exists, if not generate a 404 response
	if (access(file, F_OK) == -1)
	   error_msg = "Bad Request : File not found";
	   
 	// Verify if the file permissions are right
	
	return error_msg;
	
}


#define MAXCHAR 1024
char read_file(char *file_name){
	
	char str[MAXCHAR];
	FILE *file;	
	char *response;
	response = malloc(1024);
	
	
	file = fopen(file_name,"r");
	
	if (file == NULL){
        printf("Could not open file %s",file_name);
        return 1;
    }
    
	while (fgets(str, MAXCHAR, file) != NULL)
        sprintf(response, "%s", str);
        
        
	
	fclose(file);
	return response;
}
 

char* generate_server_response(char* request_data){

	
	puts("yo response");
//	puts(request_data);
	puts("Send the files later");
	char *file_name = "index.html";
	read_file(file_name);
	char *response = "Response file";
	return response;
}

char* generate_error_response(char* req_data){

	char* response[2000];
	//send_headers( 200, "Ok", (char*) 0, "text/html", -1, sb.st_mtime );
	snprintf(response, sizeof(response), "<!DOCTYPE html PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\" \"http://www.w3.org/TR/html4/loose.dtd\">\n\
		<html>\n\
		  <head>\n\
		    <meta http-equiv=\"Content-type\" content=\"text/html;charset=UTF-8\">\n\
		    <title>Index of %s</title>\n\
		  </head>\n\
		  <body bgcolor=\"#99cc99\">\n\
		    <pre>\n\
		 </pre>\n\
		    <h4>%s<\h4>\n\
		    <hr>\n\
		  </body>\n\
		</html>\n", req_data, req_data);
	
	(void) printf(response); 
	return response;
}


char* pull_requests_from_clients(int listening_socket, char *client_request){

	
	
	
	puts("Server is live!\n");	
	// Request from client
    	if(recv(listening_socket, client_request , 2000 , 0) < 0)
    	
	{
		perror("Failure to recieve client request ");
  	}
    	else {
    		puts("Client request received.\n");
//		puts(client_request);
  	}
		       
	
       return client_request;


}


void send_response_from_server(char *data, int listening_socket){

	printf("responding\n");
	
	
//	printf("%d\n", listening_socket);
	
	// Send response to clients	   
	if(send(listening_socket, data , strlen(data) , 0) < 0)
       {
		perror("Send failed");
				return 1;
		       }
		        else {
			    	puts("Data Sent\n");
			    	return 0;
		     } 
		    

}
/*
static void
send_headers( int status, char* title, char* extra_header, char* mime_type, off_t length, time_t mod )
    {
    time_t now;
    char timebuf[100];

    (void) printf( "%s %d %s\015\012", PROTOCOL, status, title );
    (void) printf( "Server: %s\015\012", SERVER_NAME );
    now = time( (time_t*) 0 );
    (void) strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &now ) );
    (void) printf( "Date: %s\015\012", timebuf );
    if ( extra_header != (char*) 0 )
	(void) printf( "%s\015\012", extra_header );
    if ( mime_type != (char*) 0 )
	(void) printf( "Content-Type: %s\015\012", mime_type );
    if ( length >= 0 )
	(void) printf( "Content-Length: %lld\015\012", (long long) length );
    if ( mod != (time_t) -1 )
	{
	(void) strftime( timebuf, sizeof(timebuf), RFC1123FMT, gmtime( &mod ) );
	(void) printf( "Last-Modified: %s\015\012", timebuf );
	}
    (void) printf( "Connection: close\015\012" );
    (void) printf( "\015\012" );
    }
/*

static char*
get_mime_type( char* name )
    {
    char* dot;

    dot = strrchr( name, '.' );
    if ( dot == (char*) 0 )
	return "text/plain; charset=UTF-8";
    if ( strcmp( dot, ".html" ) == 0 || strcmp( dot, ".htm" ) == 0 )
	return "text/html; charset=UTF-8";
    if ( strcmp( dot, ".xhtml" ) == 0 || strcmp( dot, ".xht" ) == 0 )
	return "application/xhtml+xml; charset=UTF-8";
    if ( strcmp( dot, ".jpg" ) == 0 || strcmp( dot, ".jpeg" ) == 0 )
	return "image/jpeg";
    if ( strcmp( dot, ".gif" ) == 0 )
	return "image/gif";
    if ( strcmp( dot, ".png" ) == 0 )
	return "image/png";
    if ( strcmp( dot, ".css" ) == 0 )
	return "text/css";
    if ( strcmp( dot, ".xml" ) == 0 || strcmp( dot, ".xsl" ) == 0 )
	return "text/xml; charset=UTF-8";
    if ( strcmp( dot, ".au" ) == 0 )
	return "audio/basic";
    if ( strcmp( dot, ".wav" ) == 0 )
	return "audio/wav";
    if ( strcmp( dot, ".avi" ) == 0 )
	return "video/x-msvideo";
    if ( strcmp( dot, ".mov" ) == 0 || strcmp( dot, ".qt" ) == 0 )
	return "video/quicktime";
    if ( strcmp( dot, ".mpeg" ) == 0 || strcmp( dot, ".mpe" ) == 0 )
	return "video/mpeg";
    if ( strcmp( dot, ".vrml" ) == 0 || strcmp( dot, ".wrl" ) == 0 )
	return "model/vrml";
    if ( strcmp( dot, ".midi" ) == 0 || strcmp( dot, ".mid" ) == 0 )
	return "audio/midi";
    if ( strcmp( dot, ".mp3" ) == 0 )
	return "audio/mpeg";
    if ( strcmp( dot, ".ogg" ) == 0 )
	return "application/ogg";
    if ( strcmp( dot, ".pac" ) == 0 )
	return "application/x-ns-proxy-autoconfig";
    return "text/plain; charset=UTF-8";
    }

*/

/* Produce value(s) */
void *boss_thread(void *param) {

	/*int i;
	
	for (i=1; i<=20; i++) {
		
		/* Insert into buffer */
/*		pthread_mutex_lock (&m);	
			if (num > BUF_SIZE) {
				exit(1);  /* overflow */
/*			}

			while (num == BUF_SIZE) {  /* block if buffer is full */
/*				pthread_cond_wait (&c_prod, &m);
			}
			
			add = (add+1) % BUF_SIZE;
			num++;
		pthread_mutex_unlock (&m);

/*		pthread_cond_signal (&c_cons);
		printf ("boss_thread: inserted %d\n", i);
		fflush (stdout);
/*	}

	printf("boss_thread quiting\n");
	fflush(stdout); */
	return 0;
}

/* Consume value(s); Note the worker_thread never terminates */
void *worker_thread(void *param) {

	int i;
	char *verification_msg;
	char *server_msg;
//	server_msg[2000] = "Server is live!\n";
	
	// Get thread ID
    	int *thread_id = pthread_self();
    
	while(1) {
 	
 		printf("Worker Thread ID %d : Thread created to listen.\n", thread_id);
 		
		pthread_mutex_lock (&m);
			if (num < 0) {
				exit(1);
			} /* underflow */

			while (num == 0) {  /* block if buffer empty */
				pthread_cond_wait (&c_cons, &m);
			}
			
    			
			// Get the next queued request
//			char socket_id[10]; 
//			socket_id[1] = buffer[rem];

			int socket_id; 
			socket_id = buffer[rem];
			
			// Check for incoming requests. 
			// If found any lock the mutex and start writing on the buffer
			char *new_request;
			new_request = malloc(1024);
			new_request = pull_requests_from_clients(socket_id, new_request);
			printf("scoket ID :::::::::::::::::: %d\n", socket_id);
			
			
			
//			printf("\nWorker Thread ID %d : Client request recieved on socket %s.\n", thread_id, new_request);
			
			 // Parse request from clients
			verification_msg = verify_request(new_request);
				       
			if (sizeof(verification_msg) > 1){
				server_msg = generate_server_response(new_request);
				//puts("Worker Thread ID %d : Server has generated a response.\n");
				//puts(server_msg);
				
				       }
		       else {
		       		puts("Worker Thread ID %d :Ignoring the erroneous request.");
				       		
		       		server_msg = generate_error_response(verification_msg);
		       		//puts("Worker Thread ID %d :Got some verification for ya");
		  		//puts(server_msg);		       		
				       }                                       
			
			//printf("Worker Thread ID %d : Sending server response to the respective client via socket %d.\n", thread_id, socket_id);
			// send the reponse to the client
			send_response_from_server(server_msg, socket_id);
			
			/* if executing here, buffer not empty so remove element */ 
			
			//close(socket_id); 
			rem = (rem+1) % BUF_SIZE;
			num--;
		pthread_mutex_unlock (&m);

		pthread_cond_signal (&c_prod);
		printf ("------------------------ Worker Thread ID %d :Consume value %d. ----------------------\n", i);  fflush(stdout);

	}
	return 0;
}



// ____________________________________________________________________________________________________________________________
// Main code for server execution



int main(int argc, char *argv[])
{
	/*char *file_name = "index.html";
	char *r;
	r  = malloc(1024);
	r[1024] =  read_file(file_name);
	printf("%s", r);
	return 0; */
	// Initialization --------------------------------------------------------

	int status;
	int bufsize = 1024;    
	char *buffer1 = malloc(bufsize);  
	
	char *verification_msg;
	char client_request[2000];
	
	 
	int listening_socket;
    	socklen_t sockaddr_size;
    	int socket_created;
	struct sockaddr_in my_server_info;
	char warning_msg[100] = " Usage : ./server port no_of_threads\n";


	// ---------------------------------------------------------- Argument checking --------------------------------------
	//!$ Change these later
	if (argc < 2) {
        	perror(warning_msg);
        	exit(1);
    	}
	
	// Port parameter
	int port;
	port = atoi(argv[1]);
	
	// no of threads
	int no_of_threads = atoi(argv[2]);
	
	
	
	pthread_t tid1, tid2;  /* thread identifiers */
	int i;

	
	// --------------------------------------  Socket programming for server and client --------------------------------------------------------
	// Create the socket, AF (address family = IP4, Sock Stream = TCP, 0=protocol IP)
        socket_created = socket(AF_INET , SOCK_STREAM , 0);
     
    	if (socket_created < 0)
    		{
    		    perror("Could not create socket");
    		    return -1;
    	}
	else {
		    printf("Socket created.\n");
		}

 	// Give the IP and port here by creating a data structure
	// Save IP and port values in the datastructure
        memset(&my_server_info, 0, sizeof(struct sockaddr_in));  
	my_server_info.sin_addr.s_addr = INADDR_ANY;			// Take any IP
	my_server_info.sin_family = AF_INET;				
	my_server_info.sin_port = htons(port);				// Port 

	// Bind the socket to the given port and an ip
	if (bind(socket_created,(struct sockaddr *)&my_server_info , sizeof(my_server_info)) < 0)
	{
	    perror("Bind failed ");
	    return -1;
	} 
	else {
		printf("Bind successful.\n");
	}

	// put the socket in listening mode
	// !$ add number of workers later
    	if (listen(socket_created, 3) == -1){
        	perror("Listening Failed!");
        	return -1;
    	}
	else {
		printf("Started listening on port %d....\n", port);
		} 
	
	// -------------------------------------------------------------------- Worker boss Thread Model -------------------------------------------	
	
	// Initiate a bunch of worker threads
	for (int thread_index=1; thread_index <= no_of_threads; thread_index++){
		
		// Thread creation of workers	
		if(pthread_create(&tid2, NULL, worker_thread, NULL) != 0) {
			fprintf(stderr, "Unable to create worker_thread thread\n");
			//exit(1);
			}
		else{
			printf("Threads created.");
		}
	}

	
	// -------------------------------------------------------------------- LIVE SERVER --------------------------------------------------------
		
	while(1){ 
	    
	    	    
	    	// Live server started running
	    	// Will keep on listening
		puts("Boss - Accepting incoming connections...");

		// size of the socket address data structure
		sockaddr_size = sizeof(struct sockaddr_in);

		// pointer to socket info datastructure &my_server_info, 
		listening_socket = accept(socket_created, (struct sockaddr*)&my_server_info, &sockaddr_size);

		// Check if socket resource is available and is accepting traffic
		if (listening_socket < 0){
			perror("Boss - Socket is not accepting the traffic.");
			continue;
		}
		else
		{
		
				/* Insert into buffer */
				pthread_mutex_lock (&m);
				
				if (num > BUF_SIZE) {
					perror("Buffer is underflowing");
					//exit(1);  /* overflow */
				}

				while (num == BUF_SIZE) {  /* block if buffer is full */
					pthread_cond_wait (&c_prod, &m);
				}
									
				
				printf("\nBoss : adding to buffer %d.\n", add);
				//buffer[add]=malloc(10);
				buffer[add] = listening_socket;
				
				printf(">>>>>>>>>>>>>>>>>>>>>>The buffer has these socket IDs : %d",buffer[add], buffer[add-1]);				
				
				add = (add+1) % BUF_SIZE;
				
				num++;
				
	
			printf("\nNew request added to the buffer.");
		
		
			
			
		pthread_mutex_unlock (&m);

		pthread_cond_signal (&c_cons);
		//printf ("producer: inserted %d\n", i);
		fflush (stdout);
		
		
			
		 
			
		    	
		    sleep(10);
		    
			    }
			    	
}

//close(listening_socket);

/* wait for created thread to exit */
//pthread_join(tid1, NULL);
//pthread_join(tid2, NULL);
printf("Parent quiting\n");


return 0;

}



