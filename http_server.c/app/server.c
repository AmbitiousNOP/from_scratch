#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>

void parse_header(char *header, char **method_variable, char **path_variable, char **temporary_UAS, char **user_agent_str, char **top_level_path, char **sub_path_var){
    char *track = header;
    char *token = strtok_r(header, " ", &track);

    int i = 0;
    while (token != NULL){
        if (i == 0){
            *method_variable = token;
        }else if (i == 1){
            *path_variable = token;
        }else if (i == 4){
            *temporary_UAS = token;
        }
        token = strtok_r(NULL, " ", &track);
        i++;
    }
    *top_level_path = strtok(*path_variable, "/");
    *sub_path_var = strtok(NULL, "\0");
    if (*temporary_UAS != NULL){
        *user_agent_str = strtok(*temporary_UAS, "\r");
    }
}


int main(){
    // Disable output buffering
	setbuf(stdout, NULL);

    // You can use print statements as follows for debugging, they'll be visible 
	//when running tests.
	printf("Logs from your program will appear here!\n");

    int server_fd, client_addr_len;
	struct sockaddr_in client_addr;
	char *header_NF = "HTTP/1.1 404 Not Found\r\n\r\n";

    // creating an ipv4 tcp socket 
	server_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (server_fd == -1) {
		printf("Socket creation failed: %s...\n", strerror(errno));
		return 1;
	}
    
    // // Since the tester restarts your program quite often, setting REUSE_PORT
	// // ensures that we don't run into 'Address already in use' errors
	int reuse = 1;
	if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &reuse, sizeof(reuse)) < 0) {
		printf("SO_REUSEPORT failed: %s \n", strerror(errno));
		return 1;
	}

    // struct to describe server address, ports, etc
	struct sockaddr_in serv_addr = { .sin_family = AF_INET ,
									.sin_port = htons(4221),
									.sin_addr = { htonl(INADDR_ANY) },
									};

    // associating the socket with local address/port
	if (bind(server_fd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) != 0) {
		printf("Bind failed: %s \n", strerror(errno));
		return 1;
    }

    // put the socket into a listening state.
	int connection_backlog = 5;
	if (listen(server_fd, connection_backlog) != 0) {
		printf("Listen failed: %s \n", strerror(errno));
		return 1;
	}

    printf("Waiting for a client to connect...\n");
	client_addr_len = sizeof(client_addr);

    while(1){
        int client_fd = accept(server_fd, (struct sockaddr *) &client_addr, &client_addr_len);
        if (client_fd == -1){
            printf("Client Connection Error;");
        }
        printf("Client Connected\n");

        int pid = fork();
        if (pid == 0){
            close(server_fd);
            //handle communications 
			char buffer[1024] = {0};
            int header = recv(client_fd, buffer, 1024, 0);
            if (header > 0){
                buffer[header] = '\0';
            } else if (header == -1){
                printf("Error recieving data from client");
            }
            printf("Size of data: %d\n", header);
            printf("Recieved Data: \n%s\n", buffer);

			// pasrsing out the http header into above variables.
			char *version = NULL, *host = NULL, *method = NULL, *path = NULL, *user_agent = NULL, *tmp_user_agent = NULL, *sub_path = NULL, *token = NULL, *tl_path = NULL; 
			parse_header(buffer, &method, &path, &tmp_user_agent, &user_agent, &tl_path, &sub_path);
			printf("Method: %s\n", method);
			printf("Path: %s\n", path);
			printf("temp UAS: %s\n", tmp_user_agent);
			printf("user agent string: %s\n", user_agent);
			printf("top level path: %s\n", tl_path);
			printf("sub path: %s\n", sub_path);

			// responding to clients request
			char resp_message[1024];
			if (path != NULL && strcmp(path, "/")==0){
				printf("Path: %s\n", path);
				snprintf(resp_message, 1024, "HTTP/1.1 200 OK\r\n\r\n");
				if (send(client_fd, resp_message, strlen(resp_message), 0) > 0){
					printf("Server Message Send Complete\n");
				}else{
					printf("Server Message Send Not Complete\n");
				}
			} else if (tl_path != NULL){
				printf("Full Path: %s\n", path);
				printf("Top Level Path: %s\n", tl_path);
				
				if (sub_path != NULL){
					printf("Lower Level Path: %s\n", sub_path);
				}
				if (strcmp(tl_path, "echo") == 0){
					snprintf(resp_message, 1024, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %ld\r\n\r\n%s\r\n", strlen(sub_path), sub_path);
					send(client_fd, resp_message, strlen(resp_message), 0);
				} else if (strcmp(tl_path, "user-agent") == 0){
					snprintf(resp_message, 1024, "HTTP/1.1 200 OK\r\nContent-Type: text/plain\r\nContent-Length: %ld\r\n\r\n%s\r\n", strlen(user_agent), user_agent);
					send(client_fd, resp_message, strlen(resp_message), 0);
				} else {
					send(client_fd, header_NF, strlen(header_NF), 0);	
				}
			} else{
				send(client_fd, header_NF, strlen(header_NF), 0);
			}
				close(client_fd);
				exit(0);
        }else if (pid < 0){
            perror("fork");
        }
        close(client_fd);
    }
    return 0;
}