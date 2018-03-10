#include <rpc/rpc.h>
#include <stdio.h>
#include "file.h"

extern __thread int errno;

readfile_res* retrieve_file_1_svc(request *req, struct svc_req *rqstp)
{
    FILE *file;
    char data[1024];
    int bytes;
    static readfile_res res;

    printf("Reading files...\n");

    file = fopen(req->name, "rb");
    if (file == NULL) {
        res.errno = errno;
        return (&res);
    }

    fseek (file, req->start, SEEK_SET);
    bytes = fread(res.readfile_res_u.chunk.data, 1, 1024, file);

    //res.readfile_res_u.chunk.data = data;
    res.readfile_res_u.chunk.bytes = bytes;

    /*
     * Return the result
     */
    res.errno = 0;
    fclose(file);
    return (&res);
}

int* send_file_1_svc(chunksend *rec, struct svc_req *rqstp)
{
    FILE *file;
    int write_bytes;
    static int result;

    printf("File name : %s\n", rec->name);
    
    //rec->name = "rida";
    
    
    file = fopen("test.jpg", "wb");
    if (file == NULL) {
        result = errno;
        printf("Null file recieved!\n");
        return &result;
        exit(0);
    }

    printf("Data written in file(%d) by the server : \n", strlen(rec->data));
    write_bytes = fwrite(rec->data, 1, strlen(rec->data), file);
    printf("Bytes written in file: %d\n", write_bytes);
    fclose(file);
    exit(0);    
    

    result = 0;
    return &result;
}
