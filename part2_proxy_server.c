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
#include<time.h>		// For creating random key numbers
 
#define SHMSZ     1024           // size of the shared memory segment



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



//_______________________________________________________________________________________________________________

int create_socket_connection(int port){

    	int client_socket;
	
    	struct sockaddr_in server_DT;
    	char client_request[1000];
    	
    	// Create the socket, AF (address family = IP4, Sock Stream = TCP, 0=protocol IP)
        client_socket = socket(AF_INET , SOCK_STREAM , 0);
     
    	if (client_socket < 0)
    	{
    		perror("Could not create socket ");
    		return 0;
    	}
	else {
		printf("Socket created %d.\n", client_socket);
	}

 	// Give the IP and port here by creating a data structure
	// Save IP and port values in the datastructure
	

	server_DT.sin_addr.s_addr = INADDR_ANY;
        server_DT.sin_family = AF_INET;
        server_DT.sin_port = htons(port);
 
    	//Connect to the server
	if (connect(client_socket, (struct sockaddr *)&server_DT , sizeof(server_DT)) < 0)
        {
        	perror("Cannot connect to the server . Error ");
        	return 0;
    	}
    	else{
     
    	printf("Client is connected to the server on port %d.\n", port);
    	return client_socket;
}
    	


}

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
	printf("The response saved in the variable is %s", response);
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
		puts(client_request);
  	}
		       
	
       return client_request;


}


int send_response_from_server(char *webserver_response, int listening_socket){

	printf("responding with data %s\n", webserver_response);
	
	
	// printf("%d\n", listening_socket);
	
	// Send response to clients	   
	if(send(listening_socket, webserver_response , strlen(webserver_response) , 0) < 0)
       {
		perror("Send failed");
		return -1;
		       }
	else {
			    	puts("Data Sent\n");
			    	return 0;
		     } 
	return 0;		    

}


int send_identifier_to_webserver(int data, int listening_socket){

	printf("Sending identifier %d to the webserver on socket ID %d.\n", data, listening_socket);

	char *key;
	key = malloc(100);
	sprintf(key,"%d", data);
	printf("stored in a character %s", key);
	// Send identifier to clients	   
	if(send(listening_socket, key , 100 , 0) < 0)
       {
		perror("Send failed : ");
		return -1;
	}
	else {
		puts("Data Sent.\n");
	    	return 0;
	    
	        } 
	        
	return 0;
		    


}


char *check_shared_memory(key_t key, char *webserver_response){

   printf("PROXY SERVER - Checking for copied response to the shared memory buffer (%d).\n", key);
   int share_seg_id;
    //key_t key;
    char *shm, *s;

 
    
    // Web server needs to find the shared segment
    if ((share_seg_id = shmget(key, SHMSZ, 0666)) < 0) {
        perror("PROXY SERVER - shmget : ");
        return -1;
       
    }else{
    
    	printf("PROXY SERVER - Segment (%d) found by the webserver.\n", key);
    }

    /*
     * Now we attach the segment to our data space.
     */
    if ((shm = shmat(share_seg_id, NULL, 0)) == (char *) -1) {
        perror("PROXY SERVER - shmat : ");
        return -1;
        
    }else{
    
    	printf("PROXY SERVER - Segment (%d) acquired.\n", key);
    }

    /*
     * Now read what the server put in the memory.
     */
    
// Reading the request here

	if (*shm==NULL){
	
		printf("PROXY SERVER - No response copied by the webserver as of yet.\n");
		// success
		return -1;
	}else
	{
    
    	printf("PROXY SERVER - This is the response copied by webserver on (%d) :  %s.\n", key, shm);
    	}
    
    	sprintf(webserver_response, "%s", shm);
	return *webserver_response;

}


//____________________________________________________________________________________________________________________


int generate_random_key(){

	int m;
	//sleep(0.0001);
	
	srand(time(NULL));
	m = rand() % 10000;
	return m;

}
 
char concatenate_int_and_string(int int_value, char char_value){

char data[400];
     sprintf(data,"%d", int_value);
     strcat(data, ":");
     strcat(data, char_value);
     return data;
}

char* concatenate_int_and_char(int int_value, char *char_value){

char *data;
data = malloc(100);

     sprintf(data,"%d", int_value);
     printf("data %s", data);
     strcat(data, ":");
     printf("data %s", data);
     strcat(data, char_value);
     printf("data %s", data);
     return data;
}


char* concatenate_int_and_int(int int_value, int int_value2, char *buffer_value){
     
     char data1[10];
     sprintf(buffer_value,"%d", int_value);
     strcat(buffer_value, ":");
     sprintf(data1,"%d", int_value2);
     strcat(buffer_value, data1);
     printf("Final concatenated value %s", buffer_value);
     
     
     return buffer_value;
}

int shared_memory_call(){

    printf("Creating threads\n");
    
    char c;
    int share_mem_id;
    key_t key;
    char *shm, *s;

   char *request;
    	request = "GET /test/backImage.html HTTP/1.1 \r\n";			

    // We will generate the key on runtime for each thread
    key = generate_random_key();
    printf("Got the key %d.\n", key);
    
    // Allocate memory space for sharing
    if ((share_mem_id = shmget(key, SHMSZ, IPC_CREAT | 0666)) < 0) {
        perror("shmget error : ");
        return -1;
    }
    else{
    	printf("Successful memory allocation.\n");
   

    // share the memory
    if ((shm = shmat(share_mem_id, NULL, 0)) == (char *) -1) {
        perror("shmat error : ");
  
    }
    else{
    
    	printf("Successful memory sharing\n");
    }
   
    s = shm;

    /*for (c =0; c <=strlen(request); c++)
        *s++ = request[c]; 
    
    *s++ = request; */
    *s = NULL;
     printf("Share memory content %s\n", shm);
    
    // Check if the webserver has placed the reponse data
    //while (*shm != '*')
     //   sleep(1);
        
    printf("changed by the client\n " );

    return key;
}
return 0;
}


// _____________________________________________________________________________________________________________________________

/* Consume value(s); Note the worker_thread never terminates */
void *worker_thread(int webserver_port) {

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
			int shared_mem_key;
			int socket_id; 
			char *buffer_data;
			
			buffer_data = malloc(100); 
			buffer_data = buffer[rem];
			
			
			
			printf("BUFFER DATA : %s\n", buffer_data);			
			sscanf(buffer_data, "%d:%d", &socket_id, &shared_mem_key);
			printf("Socket ID : %d\n", socket_id);
			printf("Shared mem key : %d\n", shared_mem_key);
			
			// Check for incoming requests from the client
			// If found any lock the mutex and start writing on the buffer
			char *new_request;
			new_request = malloc(1024);
			new_request = pull_requests_from_clients(socket_id, new_request);
			
			char *final_request;
			final_request = malloc(1024);
			
			
			final_request = concatenate_int_and_char(shared_mem_key, new_request);
			printf("socket ID :::::::::::::::::: %d\n", socket_id);
			printf("NEW REQUEST -> %s", final_request);
			
			
			//free(new_request);
			
			
			// Unlike part 1 we send the identifier and the request to the original web server
			// Here we generate the socket connection
			int webserver_socket = create_socket_connection(webserver_port);                              
			
			
			// check 20 times for a response
			int counter = 0;
			char *webserver_response;
			webserver_response = malloc(100);
			
			char *r;
			r = malloc(100);
			r = send_response_from_server(final_request, webserver_socket);
			printf("rida nae >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> %d", webserver_socket);
			
			if (webserver_socket > 0 && send_response_from_server(final_request, webserver_socket) >= 0 ){
			
				// Now wait till you recieve a signal from the webserver that it has copied the data
				
				
				recv(webserver_socket, r , 2000 , 0);
				
				*webserver_response = check_shared_memory(shared_mem_key, webserver_response);
				printf("PROXYSERVER - Response recieved  %s", webserver_response);

				while (*webserver_response = check_shared_memory(shared_mem_key, webserver_response)){
					
					if (sizeof(webserver_response) <= 0){

					// keep reading the shared memory buffer for the signal bit
					printf("Just keep swimming\n");
					counter = counter + 1;
					
					// Wait for 5 seconds here
					for (int k=0; k<=10;k++){
						printf("");
					}
					
						if (counter >= 50){
							printf("PROXYSERVER - waited for more than 50 seconds for the webserver to copy response. Exiting now");
							break;
						}else{
							printf("");
						}
					}else{
					
						printf("PROXYSERVER -Response recieved from the shared memory by the proxy server %s.", webserver_response);
						break;
					}
					
					
				}
			} else{
			
				printf("PROXYSERVER - No response recieved from the webserver. What the hell!");
				
			}
			
			printf("SErevrererwerhierhfwergwegiwherghwerghwue %s", webserver_response);
			send_response_from_server(webserver_response, socket_id);
			 
			// send this response to the client using the socket.
			
			//free(final_request);
			
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

	// discover server and port
	int webserver_port;
	webserver_port = atoi(argv[3]);


	
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
		if(pthread_create(&tid2, NULL, worker_thread, webserver_port) != 0) {
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


		// shared memory acquiring
		int shared_mem_identifier = shared_memory_call();
		printf("Memory acquisition : %d\n", shared_mem_identifier);

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
									
				// Concatenate the socket_id:key and push it on the buffer
				char *buffer_value;
				buffer_value =  malloc(100);
				
				
				buffer_value =  concatenate_int_and_int(listening_socket, shared_mem_identifier, buffer_value);
				
				printf("\nBoss : adding to buffer %s.\n", buffer_value);
				
				buffer[add] = buffer_value;
				
				printf(">>>>>>>>>>>>>>>>>>>>>>The buffer has these socket IDs : Share memory identifier %s",buffer[add], buffer[add-1]);				
				
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



