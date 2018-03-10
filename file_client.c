#include <rpc/rpc.h>
#include <stdio.h>
#include <string.h>
#include "file.h"
#define MAXCHAR 4096
extern __thread int errno;


int get_file(char *host, char *name)
{
    CLIENT *clnt;
    int total_bytes = 0, write_bytes;
    readfile_res *result;
    request req;
    FILE *file;

    req.name = name;
    req.start = 0;

    /*
     * Create client handle used for calling FTPPROG on
     * the server designated on the command line. Use
     * the tcp protocol when contacting the server.
     */
    clnt = clnt_create(host, FTPPROG, FTPVER, "tcp");
    if (clnt == NULL) {
        /*
         * Couldn't establish connection with server.
         * Print error message and stop.
         */
         clnt_pcreateerror(host);
         exit(1);
    }

    file = fopen(name, "wb");

    /*
     * Call the remote procedure readdir on the server
     */
    while (1) {
        req.start = total_bytes;
        result = retrieve_file_1(&req, clnt);
        if (result == NULL) {
            /*
             * An RPC error occurred while calling the server.
             * Print error message and stop.
             */
            clnt_perror(clnt, host);
            exit(1);
        }

        /*
         * Okay, we successfully called the remote procedure.
         */
        if (result->errno != 0) {
            /*
             * A remote system error occurred.
             * Print error message and stop.
             */
            errno = result->errno;
            perror(name);
            exit(1);
        }

        /*
         * Successfully got a chunk of the file.
         * Write into our local file.
         */
        write_bytes = fwrite(result->readfile_res_u.chunk.data, 1, result->readfile_res_u.chunk.bytes, file);
        total_bytes += result->readfile_res_u.chunk.bytes;
        if (result->readfile_res_u.chunk.bytes < MAXLEN) 
            break;
    }

    fclose(file);

    return 0;
}

int put_file(char *host, char *name)
{
    CLIENT *clnt;
    char data[4096];
    int total_bytes = 0, read_bytes;
    int *result;
    chunksend chunk;
    FILE *file;

    printf("Request to send image file %s\n", name);

    /*
     * Create client handle used for calling FTPPROG on
     * the server designated on the command line. Use
     * the tcp protocol when contacting the server.
     */
    clnt = clnt_create(host, FTPPROG, FTPVER, "tcp");
    if (clnt == NULL) {
        /*
         * Couldn't establish connection with server.
         * Print error message and stop.
         */
         clnt_pcreateerror(host);
         exit(1);
    }

    file = fopen(name, "r");
    char str[MAXCHAR];
    chunk.name = name;

    while (fgets(str, MAXCHAR, file) != NULL)
	  strcat(chunk.data, str); 
	  printf("Sending chunk..\n");
	  result = send_file_1(&chunk, clnt);

    //printf("Data saved in %s", chunk.data);
    /*
     * Call the remote procedure readdir on the server
     */

	/*
    while (1) {
    
    
    
        read_bytes = fread(data, 1, MAXLEN, file);

        total_bytes += read_bytes;

        read_bytes = fread(chunk.data, 1, MAXLEN, file);

        chunk.bytes = read_bytes;
	printf("Sending %d bytes", read_bytes);
        
        */
        
        
        
        //result = send_file_1(&chunk, clnt);

        if (result == NULL) {
            /*
             * An RPC error occurred while calling the server.
             * Print error message and stop.
             */
             printf("No file recieved from the server.\n");
            clnt_perror(clnt, host);
            exit(1);
        }
        //else {
        	//printf("Result : %s", result);
        //}

        /*
         * Okay, we successfully called the remote procedure.
         */
        if (*result != 0) {
            /*
             * A remote system error occurred.
             * Print error message and stop.
             */
            errno = *result;
            perror(name);
            exit(1);
        }

        /*
         * Successfully got a chunk of the file.
         * Write into our local file.
         */
        //if (read_bytes < MAXLEN) 
          //  break;
    //}

    fclose(file);
    printf("File closed and returning\n");
    return 0;
}

int read_command(char *host)
{
    char command[MAXLEN], filepath[MAXLEN];

    printf("> ");
    fflush(stdin);
    scanf("%s %s", command, filepath);

    if (strcmp(command, "get") == 0) {
        return get_file(host, filepath);
    } else if(strcmp(command, "put") == 0){
        return put_file(host, filepath);
    } else if(strcmp(command, "exit") == 0){
        exit(0);
    } else {
        return -1;
    }
}

int main(int argc, char *argv[])
{
   int result;

   if (argc < 2) {
        fprintf(stderr, "usage: %s host\n", argv[0]);
        exit(1);
   }

/*
   while(TRUE) {
       result = read_command(argv[1]);
   } */

   char command[MAXLEN], filepath[MAXLEN];   

   

   strcpy(filepath, argv[2]);
   put_file(argv[1], filepath);

   return 0;
}
