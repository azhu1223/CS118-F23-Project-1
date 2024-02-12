#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#include <errno.h>

/**
 * Project 1 starter code
 * All parts needed to be changed/added are marked with TODO
 */

#define BUFFER_SIZE 1024
#define CONTENT_SIZE 2000000
#define DEFAULT_SERVER_PORT 8081
#define DEFAULT_REMOTE_HOST "131.179.176.34"
#define DEFAULT_REMOTE_PORT 5001

struct server_app {
    // Parameters of the server
    // Local port of HTTP server
    uint16_t server_port;

    // Remote host and port of remote proxy
    char *remote_host;
    uint16_t remote_port;
};

// The following function is implemented for you and doesn't need
// to be change
void parse_args(int argc, char *argv[], struct server_app *app);

// The following functions need to be updated
void handle_request(struct server_app *app, int client_socket);
void serve_local_file(int client_socket, const char *path, FILE *fptr);
void proxy_remote_file(struct server_app *app, int client_socket, const char *path);

// The main function is provided and no change is needed
int main(int argc, char *argv[])
{
    struct server_app app;
    int server_socket, client_socket;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    int ret;

    parse_args(argc, argv, &app);

    server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(app.server_port);

    // The following allows the program to immediately bind to the port in case
    // previous run exits recently
    int optval = 1;
    setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == -1) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 10) == -1) {
        perror("listen failed");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d\n", app.server_port);

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_len);
        if (client_socket == -1) {
            perror("accept failed");
            continue;
        }
        
        printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
        handle_request(&app, client_socket);
        close(client_socket);
    }

    close(server_socket);
    return 0;
}

void parse_args(int argc, char *argv[], struct server_app *app)
{
    int opt;

    app->server_port = DEFAULT_SERVER_PORT;
    app->remote_host = NULL;
    app->remote_port = DEFAULT_REMOTE_PORT;

    while ((opt = getopt(argc, argv, "b:r:p:")) != -1) {
        switch (opt) {
        case 'b':
            app->server_port = atoi(optarg);
            break;
        case 'r':
            app->remote_host = strdup(optarg);
            break;
        case 'p':
            app->remote_port = atoi(optarg);
            break;
        default: /* Unrecognized parameter or "-?" */
            fprintf(stderr, "Usage: server [-b local_port] [-r remote_host] [-p remote_port]\n");
            exit(-1);
            break;
        }
    }

    if (app->remote_host == NULL) {
        app->remote_host = strdup(DEFAULT_REMOTE_HOST);
    }
}

void handle_request(struct server_app *app, int client_socket) {
    char buffer[BUFFER_SIZE];
    ssize_t bytes_read;

    // Read the request from HTTP client
    // Note: This code is not ideal in the real world because it
    // assumes that the request header is small enough and can be read
    // once as a whole.
    // However, the current version suffices for our testing.
    bytes_read = recv(client_socket, buffer, sizeof(buffer) - 1, 0);
    if (bytes_read <= 0) {
        return;  // Connection closed or error
    }

    buffer[bytes_read] = '\0';
    // copy buffer to a new string
    char *request = malloc(strlen(buffer) + 1);
    strcpy(request, buffer);

    // printf("Header: %s", request);

    // TODO: Parse the header and extract essential fields, e.g. file name
    // Hint: if the requested path is "/" (root), default to index.html
    char* file_name;

    char* header_token = strtok(request, " \n\t");
    header_token = strtok(NULL, " \n\t");

    if (strcmp(header_token, "/") == 0)
    {
        file_name = "index.html";
    }

    else
    {
        file_name = header_token + 1;
    }

    char* percent_escape_pointer;

    while ((percent_escape_pointer = strstr(file_name, "%20")))
    {
        percent_escape_pointer[0] = ' ';

        for (percent_escape_pointer++; *percent_escape_pointer; percent_escape_pointer++)
        {
            percent_escape_pointer[0] = percent_escape_pointer[2];
        }
    }

    while ((percent_escape_pointer = strstr(file_name, "%25")))
    {
        percent_escape_pointer[0] = '%';

        for (percent_escape_pointer++; *percent_escape_pointer; percent_escape_pointer++)
        {
            percent_escape_pointer[0] = percent_escape_pointer[2];
        }
    }

    printf("Requested file name: %s\n", file_name);

    if (strcmp(file_name, "output2.ts") == 0)
    {
        printf("This is output2!\n");
    }

    printf("Opening %s\n", file_name);

    FILE* fptr = fopen(file_name, "rb");
    
    // TODO: Implement proxy and call the function under condition
    // specified in the spec
    if (fptr) 
    {
        printf("Found %s. Sending file.\n", file_name);
        serve_local_file(client_socket, file_name, fptr);
        fclose(fptr);
    } 
    
    else 
    {
        printf("Did not find %s locally. Asking remote for it.\n", file_name);
        proxy_remote_file(app, client_socket, buffer);
    }

    free(request);
}

void serve_local_file(int client_socket, const char *path, FILE *fptr) {
    // TODO: Properly implement serving of local files
    // The following code returns a dummy response for all requests
    // but it should give you a rough idea about what a proper response looks like
    // What you need to do 
    // (when the requested file exists):
    // * Open the requested file
    // * Build proper response headers (see details in the spec), and send them
    // * Also send file content
    // (When the requested file does not exist):
    // * Generate a correct response

    unsigned char* content_type;
    unsigned char content[CONTENT_SIZE];
    unsigned int content_length = 0;
    int precontent_response_length;

    unsigned char response[2 * CONTENT_SIZE] = "";
    
    char* file_extension = strrchr(path, '.');

    if (file_extension)
    {
        if (strcmp(file_extension, ".html") == 0)
        {
            printf("Detected file extension .html\n");
            content_type = "Content-Type: text/html; charset=UTF-8\r\n";
        }

        else if (strcmp(file_extension, ".txt") == 0)
        {
            printf("Detected file extension .txt\n");
            content_type = "Content-Type: text/plain; charset=UTF-8\r\n";
        }

        else if (strcmp(file_extension, ".jpg") == 0)
        {
            printf("Detected file extension .jpg\n");
            content_type = "Content-Type: image/jpeg\r\n";
        }

        else
        {
            printf("Detected binary file\n");
            content_type = "Content-Type: application/octet-stream\r\n";
        }
    }

    else
    {
        printf("Detected binary file\n");
        content_type = "Content-Type: application/octet-stream\r\n";
    }

    printf("Reading %s\n", path);
    content_length = fread(content, sizeof content[0], CONTENT_SIZE, fptr);

    char content_length_text[BUFFER_SIZE];
    sprintf(content_length_text, "Content-Length: %u\r\n\r\n", content_length);

    strcat(response, "HTTP/1.0 200 OK\r\n");
    strcat(response, content_type);
    strcat(response, content_length_text);

    precontent_response_length = strlen(response);

    memcpy(response + strlen(response), content, content_length);

    send(client_socket, response, precontent_response_length + content_length, 0);
}

void proxy_remote_file(struct server_app *app, int client_socket, const char *request) {
    // TODO: Implement proxy request and replace the following code
    // What's needed:
    // * Connect to remote server (app->remote_server/app->remote_port)
    // * Forward the original request to the remote server
    // * Pass the response from remote server back
    // Bonus:
    // * When connection to the remote server fail, properly generate
    // HTTP 502 "Bad Gateway" response

    printf("Entered proxy remote file.\n");
    unsigned char response[3 * CONTENT_SIZE];
    struct sockaddr_in remote_addr;

    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(app->remote_port);

    printf("Creating remote socket.\n");

    int remote_socket = socket(AF_INET, SOCK_STREAM, 0);

    printf("Converting address string to binary.\n");
    if (inet_pton(AF_INET, app->remote_host, &remote_addr.sin_addr)<=0)
    {
        printf("\n inet_pton error occured\n");
        exit(EXIT_FAILURE);
    } 

    int header_bytes_read;
    int total_content_bytes_read;

    printf("Connecting to remote.\n");
    if (connect(remote_socket, (struct sockaddr*)&remote_addr, sizeof (remote_addr)) == -1)
    {
        char* header = "HTTP/1.0 502 BAD GATEWAY\r\n\r\n";
        strcpy(response, header);

        header_bytes_read = strlen(header);
    }

    else
    {
        printf("Sending request to remote.\n");
        send(remote_socket, request, strlen(request), 0);

        printf("Reading remote response.\n");
        header_bytes_read = recv(remote_socket, response, CONTENT_SIZE, 0);

        //response[header_bytes_read] = '\0';

        // printf("Header: %s\n", response);

        unsigned char* content = response + header_bytes_read;

        int content_bytes_read = recv(remote_socket, content, CONTENT_SIZE, 0);

        total_content_bytes_read = content_bytes_read;

        while (content_bytes_read > 0)
        {
            printf("Read %u bytes of content.\n", content_bytes_read);
            content += content_bytes_read;
            content_bytes_read = recv(remote_socket, content, CONTENT_SIZE, 0);
            total_content_bytes_read += content_bytes_read;
        }

        printf("Content size: %u\n", total_content_bytes_read);

        //printf("HTML Message: %s\n", response);

        printf("Forward response from remote to client.\n");

        close(remote_socket);
    }

    send(client_socket, response, header_bytes_read + total_content_bytes_read, 0);
}
