#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using std::cout;
using std::cerr;


#define SERVER_PORT 8080 //not forwarded in docker_config, yet
#define MAX_BACKLOG 5

const int port=4277;
const char ip[]="127.0.0.1"; // for local host
//const ip[]="0.0.0.0"; // for allowing all incomming connection from internet
struct clientDetails{
    int32_t clientfd;  // client file descriptor
    int32_t serverfd;  // server file descriptor
    std::vector<int> clientList; // for storing all the client fd
    clientDetails(void){ // initializing the variable
        this->clientfd=-1;
        this->serverfd=-1;
    }
};

int main(int, char**){
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(server_fd == 0){
        cerr << "Failed to create socket\n";
        return 1;
    }

    //Extra options
    int opt = 1;
    if(setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))){
        cerr << "setsockopt failed\n";
        close(server_fd);
        return 1;
    }

    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    
    int status = bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    if(status < 0){
        cerr << "Bind failed\n";
        close(server_fd);
        return 1;
    }

    if(listen(server_fd, MAX_BACKLOG) < 0){
        cerr << "listen failed\n";
        close(server_fd);
        return 1;
    }

    cout << "Server listing on port " << SERVER_PORT << "...\n";

    while(true){

    }

    close(server_fd);
    return 0;
}
