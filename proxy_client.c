#include<stdio.h> //printf
#include<string.h>    //strlen
#include<sys/socket.h>    //socket
#include<arpa/inet.h> //inet_addr
#include<stdlib.h> 
 
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
 
 
char* send_requests_from_clients(char* listening_socket, char* data, int thread_no){

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
		printf("CLIENT - send_requests_from_clients - %d - Request sent to the server. \n", thread_no);
		//puts(thread_id);
		//puts("-----------");

	}

	return request_status;		 
} 
 
char* pull_response_from_server(int listening_socket, char *server_response){
	
	// Response from sercer
    	if(recv(listening_socket, server_response , 2000 , 0) < 0)
    	
	{
		perror("Failure to recieve client request ");
  	}
    	else {
    		printf("CLIENT - pull_response_from_server - %d - Response from server recieved.\n");
		//printf(server_response);
  	}
		       
	
       return server_response;


}
 
 
void *request_generator(int port)
{   
    
    int *thread_request_number = 5;  
    
    // Get thread ID
    int *thread_id = pthread_self();
    
    int client_socket;
    	struct sockaddr_in server_DT;
    	
    	
    	printf("CLIENT - request_generator - %u - Thread Created \n", thread_id);
    	// Create the socket, AF (address family = IP4, Sock Stream = TCP, 0=protocol IP)
        client_socket = socket(AF_INET , SOCK_STREAM , 0);
     
    	if (client_socket < 0)
    	{
    		perror("Could not create socket ");
    		//return -1;
    	}
	else {
		printf("Socket created %d.\n", client_socket);
	}

 	
	server_DT.sin_addr.s_addr = inet_addr("127.0.0.1");
        server_DT.sin_family = AF_INET;
        server_DT.sin_port = htons(port);
 
    	//Connect to the server
	if (connect(client_socket, (struct sockaddr *)&server_DT , sizeof(server_DT)) < 0)
        {
        	perror("Cannot connect to the server . Error ");
        	//return 1;
    	}
     
    	puts("Connected to server !\n");
  
    
    
    
    // Created a array which contains requests
    // The threads will randomy pick a request
    const char *request_array[] = {
    
    "GET /test/backImage.html HTTP/1.1 \nHost: www.test.com\nAccept: image/gif, image/jpeg, */*\nAccept-Language: en-us\nAccept-Encoding: gzip, deflate\nUser-Agent: Mozilla/4.0 (compatible; MSIE 6.0\nWindows NT 5.1",
    "GET /test/image.html HTTP/1.1 \nHost: www.test.com\nAccept: image/gif, image/jpeg, */*\nAccept-Language: en-us\nAccept-Encoding: gzip, deflate\nUser-Agent: Mozilla/4.0 (compatible; MSIE 6.0\nWindows NT 5.1",
    "GET /test/image2.html HTTP/1.1 \nHost: www.test.com\nAccept: image/gif, image/jpeg, */*\nAccept-Language: en-us\nAccept-Encoding: gzip, deflate\nUser-Agent: Mozilla/4.0 (compatible; MSIE 6.0\nWindows NT 5.1",
    "GET /test/simple.html HTTP/1.1 \nHost: www.test.com\nAccept: image/gif, image/jpeg, */*\nAccept-Language: en-us\nAccept-Encoding: gzip, deflate\nUser-Agent: Mozilla/4.0 (compatible; MSIE 6.0\nWindows NT 5.1"
         };
    
    
    // Communication channel procedures

    
    for (int i=0; i < thread_request_number ; i++){
	    	    
	    // Generates a random number between 0 and 3
	    char *client_request = request_array[rand() % 4];
	    send_requests_from_clients(client_socket, client_request, thread_id) ;
	    printf("\nCLIENT - request_generator - %u - Request no. %d on socket ID %d.\n",thread_id, i, client_socket);
	    int counter = 0;
	    
//	    while (counter < 10)
//	    	{				
	    sleep(1);
	    // Send request on the server
	    if (send_requests_from_clients(client_socket, client_request, thread_id) > 0 ){
	    	
	    	printf("\nCLIENT - request_generator - %u - Client request sent. Waiting for response.\n",thread_id);
	    	// Pull response from the server sent on the communciation channel
	    		   	    	
	    	
		ssize_t bytes_recieved;
	    	
	    	// Receive a reply from the server
	    	char *new_response;
		new_response = malloc(1024);
	    	bytes_recieved = recv(client_socket, new_response , 2000 , 0);
	    	
		    	if(bytes_recieved < 0)
			{
			    perror("\nCLIENT - request_generator  - Response from server failed : ");
			    
			}
			else{
			   
			    printf("\nCLIENT - request_generator - %u - Server reply : %s\n",thread_id, new_response);
			    printf("\nCLIENT - request_generator - %u - Bytes recieved %d \n", thread_id, bytes_recieved); 

  	    // Concatenate the threadid_socketID_bytes
		char *file_name, *temp;
		file_name = malloc(20);
		temp = malloc(10);
		
	   sprintf(file_name, "%u", thread_id);
	    strcat(file_name, "_");
	    sprintf(temp, "%d", client_socket);
	    strcat(file_name, temp);
	    strcat(file_name, "_");
	   sprintf(temp, "%d", bytes_recieved);
	    strcat(file_name, temp);
	    printf("idhr tk %s", file_name);
   	    write_to_file(file_name, new_response);
   	    
   	    free(new_response);
		
			}	
			
		
		
		break;
		printf("\nCLIENT - request_generator - %u - Retrying request %d",thread_id, counter); 
	    	counter = counter + 1;    
	    
	    } else {
	    
	    	perror("CLIENT - request_generator - Error while sending request to server : ");
	    } 
	    }
	    printf("\nCLIENT - request_generator - %u -  ---------------------- Communication complete ---------------------\n", thread_id);
	    
		//break;
	    // Wait for a few minutes
	    //sleep(5);
	    
	   // }
	    
	    // close the connection
	   close(client_socket);
    
    return 0;
    
   }


 
// ____________________________________________________________________________________________________________________________
// Main code for server_DT execution

int main(int argc , char *argv[])
{

    	// Initialization --------------------------------------------------------
    	
    	int port;
    	int no_of_threads = atoi(argv[2]);
	int repeat_threading = 0;
	
	
    	char client_request[1000] , server_response[2000];
    	
    	// Thread related initialization
    	pthread_t tid;
    	pthread_t thread_array[no_of_threads];
    	// Give the IP and port here by creating a data structure
	// Save IP and port values in the datastructure
	// Port parameter
	port = atoi(argv[1]);
    

   	// Client communication with the server
	
    		// This will repeat one time only. Change the repeat_thread variable 
    		//to allow infinite client requests and add an argument
    		
    		
    		
			for (int i=0; i < no_of_threads; i++){
			       
			       //int listening_socket = connect(client_socket, (struct sockaddr*)&server_DT,  sizeof(server_DT));
			       pthread_create(&thread_array[i], NULL, request_generator, port);
		    		
		    		
			}
		
		for (int y=0; y < no_of_threads; y++){

			pthread_join(thread_array[y], NULL);
			
			}
     		//puts("Multithreaded client request sessions complete.");

     
   // close(client_socket);
    return 0;
}
