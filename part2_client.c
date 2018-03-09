#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<stdlib.h> 
#include<time.h>
 
 
 
char* send_requests_from_clients(char* listening_socket, char* data){

	// Function to send request by connecting to the server socket
	// Get thread ID
	int *thread_id = pthread_self();
	
	int request_status = send(listening_socket,data ,strlen(data) , 0);
	
	if( request_status < 0)
	{
		perror("Sending failed");
		return -1;
	}
	else
	{
		printf("Request sent to the server by thread ID %d \n", thread_id);
		//puts(thread_id);
		//puts("-----------");

	}

	return request_status;		 
} 
 

 
int write_to_file(char *file_name, char* data){
	FILE *fp;
	fp=fopen(file_name,"w");
	int status = fputs(data, fp);
	if (status == EOF) {
	    perror("File write error : ");
	}	
	fclose(fp);
	return 0;
}
 
void *request_generator(int port)
{
	
    sleep(1);
    int *thread_request_number = 5;  
    char server_response[2000];
    
    // Get thread ID
    int *thread_id = pthread_self();
  
    printf("Thread ID %d :\n", thread_id);
    
    
    // Created a array which contains requests
    // The threads will randomy pick a request
    const char *request_array[] = {
    
    "GET http://localhost:8080/test/backImage.html HTTP/1.1 Host: www.test.comAccept: image/gif, image/jpeg, */*Accept-Language: en-usAccept-Encoding: gzip, deflateUser-Agent: Mozilla/4.0 (compatible; MSIE 6.0Windows NT 5.1\r\n",
    "GET http://localhost:8080/test/image.html HTTP/1.1 Host: www.test.comAccept: image/gif, image/jpeg, */*Accept-Language: en-usAccept-Encoding: gzip, deflateUser-Agent: Mozilla/4.0 (compatible; MSIE 6.0Windows NT 5.1\r\n",
    "GET http://localhost:8080/test/image2.html HTTP/1.1 Host: www.test.comAccept: image/gif, image/jpeg, */*Accept-Language: en-usAccept-Encoding: gzip, deflateUser-Agent: Mozilla/4.0 (compatible; MSIE 6.0Windows NT 5.1\r\n",
    "GET http://localhost:8080/test/simple.html HTTP/1.1 Host: www.test.comAccept: image/gif, image/jpeg, */*Accept-Language: en-usAccept-Encoding: gzip, deflateUser-Agent: Mozilla/4.0 (compatible; MSIE 6.0Windows NT 5.1\r\n"
         };
    
    // Generates a random number between 0 and 3
    char *client_request = request_array[rand() % 4];
	    
    // Here we generate the socket connectio
    int listening_socket = create_socket_connection(port);
	    
    int counter = 0;
    ssize_t bytes_recieved ;
    
    //Send some data
    for (int i=0; i < thread_request_number ; i++){
	    	    
	    printf("Request no. %d from thread ID %d and client socket %d\n", i, thread_id, listening_socket);
	    // Send request on the server
	    send_requests_from_clients(listening_socket, client_request);
	  
	    bytes_recieved = recv(listening_socket, server_response , 2000 , 0);
	    if (bytes_recieved  > 0 ){
	    	counter = counter + 1;
		puts("Server reply :");
		puts(server_response);
		printf("Bytes recieved %d \n", bytes_recieved); 
		
		// wait for 60 tries
		//if (counter > 60){
		//	break;
		//}
		// Concatenate the threadid_socketID_bytes
		char *file_name, *temp;
		file_name = malloc(20);
		temp = malloc(10);
		
	   sprintf(file_name, "%u", thread_id);
	    strcat(file_name, "_");
	    sprintf(temp, "%d", listening_socket);
	    strcat(file_name, temp);
	    strcat(file_name, "_");
	   sprintf(temp, "%d", bytes_recieved);
	    strcat(file_name, temp);
	    printf("CLIENT SERVER - %u - File name :  %s\n", thread_id, file_name);
   	    write_to_file(file_name, server_response);
   	    printf("CLIENT SERVER - %u - Transaction complete.\n", thread_id);
	    
	    
	    }
	    else{
	    	printf("No bytes recieved.");
	    }
	    //Receive a reply from the server
   		printf("out");

	
	    // Wait for a few minutes
	    //sleep(5);
	    break;
	    }
	    
    

    close(listening_socket);
    
    printf("All requests complete from from thread ID %d . (Connection closed)\n", thread_id);
    return 0;
    
   }


int generate_random_port_number(){

	int m;
	//sleep(0.0001);
	
	srand(time(NULL));
	m = rand() % 10000;
	return m;

}
 
int create_socket_connection(int port){

    	int client_socket;
	
    	struct sockaddr_in server_DT;
    	char client_request[1000];
    	
    	// Create the socket, AF (address family = IP4, Sock Stream = TCP, 0=protocol IP)
        client_socket = socket(AF_INET , SOCK_STREAM , 0);
     
    	if (client_socket < 0)
    	{
    		perror("Could not create socket ");
    		return 1;
    	}
	else {
		printf("Socket created %d.\n", client_socket);
	}

 	// Give the IP and port here by creating a data structure
	// Save IP and port values in the datastructure
	

	server_DT.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_DT.sin_family = AF_INET;
        server_DT.sin_port = htons(port);
 
    	//Connect to the server
	if (connect(client_socket, (struct sockaddr *)&server_DT , sizeof(server_DT)) < 0)
        {
        	perror("Cannot connect to the server . Error ");
        	return 1;
    	}
     
    	printf("Client is connected to the server on port %d.\n", port);

    	return client_socket;


}
// ____________________________________________________________________________________________________________________________
// Main code for server_DT execution

int main(int argc , char *argv[])
{

    	// Initialization --------------------------------------------------------
	
	// Port parameter
	//port = generate_random_port_number();
	int port = atoi(argv[1]);
    	int no_of_threads = atoi(argv[2]);
	int repeat_threading = 0;
	printf("%d", port);
    	// Thread related initialization
    	pthread_t tid;

	
   	// Client communication with the server
	// This will repeat one time only. Change the repeat_thread variable 
    	//to allow infinite client requests and add an argument
    	if (repeat_threading == 0) {
		for (int i=0; i < no_of_threads; i++){
		
			       pthread_create(&tid, NULL, request_generator, port);
		    
			}
		}
		
	// Do not allow more than one client request session
	repeat_threading = repeat_threading + 1;
		
		
	pthread_join(tid, NULL);
     	//puts("Multithreaded client request sessions complete.");
     	

    //close(client_socket);
    return 0;
}
