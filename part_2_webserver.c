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

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

#define SHMSZ     1024


#define BUF_SIZE 3000		/* Size of shared buffer */

int buffer[BUF_SIZE];  	/* shared buffer */
int add = 0;  			/* place to add next element */
int rem = 0;  			/* place to remove next element */
int num = 0;  			/* number elements in buffer */

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;  	/* mutex lock for buffer */
pthread_cond_t c_cons = PTHREAD_COND_INITIALIZER; /* worker_thread waits on this cond var */
pthread_cond_t c_prod = PTHREAD_COND_INITIALIZER; /* boss_thread waits on this cond var */

//void *boss_thread (void *param);
//void *worker_thread (void *param);


char concatenate_int_and_string(int int_value, char char_value){

char data[100];
     sprintf(data,"%d", int_value);
     strcat(data, char_value);
     return data;
}
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


#define MAXCHAR 4096
char read_file(char *file_name, char *response){

	int *thread_id = pthread_self();
	
	char str[MAXCHAR];
	FILE *file;	
	//char *response;
	//response = malloc(1024);
	
	
	file = fopen(file_name,"r");
	
	if (file == NULL){
        printf("WEBSERVER - %u - Could not open file %s\n",thread_id, file_name);
	response = "";
        return *response;
    }
    
    	char *temp;
    	temp = malloc(100);
    	
    	//fgets(str, MAXCHAR, file);
    	//printf("Wholesome kanola file %s", str);
	
	while (fgets(str, MAXCHAR, file) != NULL)
	
        //sprintf(temp, "%s", str);
        //printf("Intermediate file %s", temp);
        strcat(response, str);
        //printf("Intermediate file after modification %s", response);        
        
	
	fclose(file);
	printf("WEBSERVER - %u - File read %s\n", thread_id, response);
	return *response;
}
 

char* generate_server_response(char* request_data){

	char *response;
	response = malloc(1024);
	int *thread_id = pthread_self();
	
	printf("WEBSERVER - %u - Client's request %s\n", thread_id, request_data);

	// Parse the file name
	char file_name[100], method[10000], path[10000], protocol[10000];
	sscanf(request_data, "%[^ ] %[^ ] %[^ ]", method, path, protocol);

	sscanf(path, "http://localhost:8080/test/%[^$]", file_name);
	
	printf("WEBSERVER - %u - Requested file name %s from path %s.\n",thread_id, file_name, path);
	
	char *new_file ;
	new_file = malloc(16);
	strcpy(new_file, "test/");
	strcat(new_file, file_name);
	
	*response = read_file(new_file, response);
		
	if (sizeof(response) <= 0){
		printf("WEBSERVER - %u - 404 File %s not found.\n", thread_id, file_name);
		
	}
	
	printf("WEBSERVER - %u - Response => %s\n", thread_id, response);
	return response;
}

char* generate_error_response(char* req_data){

	int *thread_id = pthread_self();

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
	
	(void) printf("WEBSERVER - %u - Response %s",thread_id, response); 
	return response;
}


char* pull_requests_from_clients(int listening_socket, char *client_request){

	
	int *thread_id = pthread_self();
	
	printf("WEBSERVER - %u - Server is ready to pull from the client.!\n", thread_id);
		
	// Request from client
    	if(recv(listening_socket, client_request , 2000 , 0) < 0)
    	
	{
		perror("Failure to recieve client request ");
  	}
    	else {
    		printf("WEBSERVER - %u - Client request received.\n", thread_id);

  	}
		       
       return client_request;


}


int pull_identifier_from_clients(int listening_socket){

	int identifier;
	
	puts("Server is live!\n");	
	// Request from client
    	if(recv(listening_socket, identifier , 2000 , 0) < 0)
    	
	{
		perror("Failure to recieve client request ");
  	}
    	else {
    		printf("Client request received.\n ------------\n %s ------------- \n", identifier);
//		puts(client_request);
  	}
	
       return identifier;


}

int send_response_from_server(char *data, int listening_socket){

	// Get thread ID
    	int *thread_id = pthread_self();

	printf("WEBSERVER - %u - Server responding by sending data %s\n", thread_id, data);
	
	
//	printf("%d\n", listening_socket);
	
	// Send response to clients	   
	if(send(listening_socket, data , strlen(data) , 0) < 0)
       {
		perror("Send failed");
				return 1;
		       }
		        else {
			    	printf("WEBSERVER - %u - Data %s Sent\n", thread_id, data);
			    	return 0;
		     } 
		    

}

// _______________________________________________________________________________________________________________________


int copy_response_to_shared_memory(key_t key, char *response){

	printf("WEBSERVER - Copying the response to the shared memory buffer.\n");
   int share_seg_id;
    //key_t key;
    char *shm, *s;

 
    
    // Web server needs to find the shared segment
    if ((share_seg_id = shmget(key, SHMSZ, 0666)) < 0) {
        perror("WEBSERVER - shmget : ");
        return -1;
       
    }else{
    
    	printf("WEBSERVER - Segment %d found by the webserver.\n", key);
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = shmat(share_seg_id, NULL, 0)) == (char *) -1) {
        perror("WEBSERVER - shmat : ");
        return -1;
        
    }else{
    
    	printf("WEBSERVER - Segment %d acquired .\n", key);
    }

    /*
     * Now read what the server put in the memory.
     */
    printf("WEBSERVER - We have data in the shared memory (%d) %s.\n", key, shm);

    // Reading the request here
    for (s = shm; *s != NULL; s++)
        putchar(*s);
        
        
    putchar('\n');

     
     s = shm;
     
    for (int c =0; c <=strlen(response); c++)
        *s++ = response[c]; 
    
    *s++ = response; 
    *s = NULL;
   
     shm = response;
    
    printf("WEBSERVER - We have daata in the shared memory %s.\n", shm);
    
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
 	
 		printf("WEBSERVER - %u - Worker Thread ID %d : Thread created to listen.\n", thread_id, thread_id);
 		
		pthread_mutex_lock (&m);
			if (num < 0) {
				exit(1);
			} /* underflow */

			while (num == 0) {  /* block if buffer empty */
				pthread_cond_wait (&c_cons, &m);
			}
			
    			
			// Get the next queued request

			int socket_id; 
			socket_id = buffer[rem];
			

			int shared_mem_identifier;
			//shared_mem_identifier = malloc(100);
			//shared_mem_identifier = pull_identifier_from_clients(socket_id);
			//printf("WEBSERVER - Shared key pulled from proxy server : %d", shared_mem_identifier);
			
			// Check for incoming requests. 
			// If found any lock the mutex and start writing on the buffer
			char *new_request;
			char *final_request;
			new_request = malloc(1024);
			final_request = malloc(1024);
			new_request = pull_requests_from_clients(socket_id, new_request);
			printf("WEBSERVER - %u - socket ID = %d\n", thread_id, socket_id);
			
			sscanf(new_request, "%d:%99[^\r\n]", &shared_mem_identifier, final_request);
			printf("Socket ID :::::::::::::::::: %d\nResponse recieved : %s\n", shared_mem_identifier, final_request);
//			printf("\nWorker Thread ID %d : Client request recieved on socket %s.\n", thread_id, new_request);
			
			 // Parse request from clients
			verification_msg = verify_request(final_request);
				       
			if (sizeof(verification_msg) > 1){
				server_msg = generate_server_response(final_request);
				//puts("Worker Thread ID %d : Server has generated a response.\n");
				//puts(server_msg);
				
				       }
		       else {
		       		printf("WEBSERVER - %u - Worker Thread ID %d :Ignoring the erroneous request.\n", thread_id, thread_id);
				       		
		       		server_msg = generate_error_response(verification_msg);
		       		//puts("Worker Thread ID %d :Got some verification for ya");
		  		//puts(server_msg);		       		
				       }                                          
			
			printf("Worker Thread ID %d : Sending server response to the shared memory buffer %s.\n", thread_id, server_msg);
			// send the reponse to the client
			printf("SHARED : %d", shared_mem_identifier);
			int status = copy_response_to_shared_memory(shared_mem_identifier, server_msg);
			
			
			if (status >= 0){
				send_response_from_server("success", socket_id);
						
				
				}
			else {
				printf("Error : Cannot copy response to the shared memory.\n");
			}
			
			/* if executing here, buffer not empty so remove element */ 
			
			//close(socket_id); 
			rem = (rem+1) % BUF_SIZE;
			num--;
		pthread_mutex_unlock (&m);

		pthread_cond_signal (&c_prod);
		printf ("WEBSERVER - %u - Worker Thread ID %d : Consume value %d. ----------------------\n",thread_id, i); 
		fflush(stdout);

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
    		    perror("WEBSERVER - Could not create socket");
    		    return -1;
    	}
	else {
		    printf("WEBSERVER - Socket created.\n");
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
		printf("WEBSERVER - Bind successful.\n");
	}

	// put the socket in listening mode
	// !$ add number of workers later
    	if (listen(socket_created, 3) == -1){
        	perror("WEBSERVER - Listening Failed!");
        	return -1;
    	}
	else {
		printf("WEBSERVER - Started listening on port %d....\n", port);
		} 
	
	// -------------------------------------------------------------------- Worker boss Thread Model -------------------------------------------	
	
	// Initiate a bunch of worker threads
	for (int thread_index=1; thread_index <= no_of_threads; thread_index++){
		
		// Thread creation of workers	
		if(pthread_create(&tid2, NULL, worker_thread, NULL) != 0) {
			fprintf(stderr, "WEBSERVER - Unable to create worker_thread thread\n");
			//exit(1);
			}
		else{
			printf("WEBSERVER - Threads created.\n");
		}
	}

	
	// -------------------------------------------------------------------- LIVE SERVER --------------------------------------------------------
		
	while(1){ 
	    
	    	    
	    	// Live server started running
	    	// Will keep on listening
		puts("WEBSERVER - Boss - Accepting incoming connections...\n");

		// size of the socket address data structure
		sockaddr_size = sizeof(struct sockaddr_in);

		// pointer to socket info datastructure &my_server_info, 
		listening_socket = accept(socket_created, (struct sockaddr*)&my_server_info, &sockaddr_size);

		// Check if socket resource is available and is accepting traffic
		if (listening_socket < 0){
			perror("WEBSERVER - Boss - Socket is not accepting the traffic.\n");
			continue;
		}
		else
		{
		
				/* Insert into buffer */
				pthread_mutex_lock (&m);
				
				if (num > BUF_SIZE) {
					perror("WEBSERVER - Buffer is underflowing");
					//exit(1);  /* overflow */
				}

				while (num == BUF_SIZE) {  /* block if buffer is full */
					pthread_cond_wait (&c_prod, &m);
				}
									
				
				printf("\nWEBSERVER - Boss : adding to buffer %d.\n", add);
				//buffer[add]=malloc(10);
				buffer[add] = listening_socket;
				
				printf("WEBSERVER - The buffer has these socket ID : %d",buffer[add], buffer[add-1]);				
				
				add = (add+1) % BUF_SIZE;
				
				num++;
				
	
			printf("\nWEBSERVER - New request added to the buffer.");
		
		
			
			
		pthread_mutex_unlock (&m);

		pthread_cond_signal (&c_cons);
		//printf ("producer: inserted %d\n", i);
		fflush (stdout);
		
		    
			    }
			    	
}

//close(listening_socket);

/* wait for created thread to exit */
//pthread_join(tid1, NULL);
//pthread_join(tid2, NULL);
printf("Parent quiting\n");


return 0;

}



