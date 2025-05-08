#include <iostream>
#include <vector>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
using std::cout;
using std::cerr;


#define SERVER_PORT 8080 //not forwarded in docker_config, yet
#define MAX_BACKLOG 5

const int port=8080;
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
    auto client= new clientDetails();
    client->serverfd = socket(AF_INET, SOCK_STREAM, 0);
    if(client->serverfd <= 0){
        cerr << "Failed to create socket\n";
        delete client;
        exit(1);
    }
    cout << "socket created\n";

    //Extra options
    int opt = 1;
    if(setsockopt(client->serverfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0){
        cerr << "setsockopt failed\n";
        delete client;
        exit(2);
    }

    struct sockaddr_in server_addr;
    // memset(&server_addr, 0, sizeof(server_addr));
    // server_addr.sin_family = AF_INET;
    // server_addr.sin_port = htons(SERVER_PORT);
    // inet_pton(AF_INET, ip, &server_addr.sin_addr);
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(port);
    inet_pton(AF_INET, ip, &server_addr.sin_addr);
    cout << "[DB] serverfd: " << client->clientfd << "\n size of struct: " << sizeof(server_addr) << '\n';
    int status = bind(client->serverfd, (struct sockaddr *)&server_addr, sizeof(server_addr));
    cout << "[DB] Status: " << status << '\n';
    if(status < 0){
        cerr << "Bind failed\n";
        delete client;
        exit(3);
    }

    if(listen(client->serverfd, MAX_BACKLOG) < 0){
        cerr << "listen failed\n";
        delete client;
        exit(4);
    }
    cout << "Server listing on port " << SERVER_PORT << "...\n";

    fd_set readfds;
    size_t  valread;
    int maxfd;
    int sd=0;
    int activity;
    while(true){
        std::cout<<"waiting for activity\n";
        FD_ZERO(&readfds);
        FD_SET(client->serverfd, &readfds);
        maxfd=client->serverfd;
        // copying the client list to readfds
        // so that we can listen to all the client
        for(auto sd:client->clientList){
                FD_SET(sd, &readfds);
                if (sd>maxfd){
                    maxfd=sd;
                }
        }
        //
        if (sd>maxfd){
                maxfd=sd;
        }

        activity=select(maxfd+1, &readfds, NULL, NULL, NULL);
        if (activity<0){
                std::cerr<<"select error\n";
                continue;
        }

        if (FD_ISSET(client->serverfd, &readfds)) {
            client->clientfd = accept(client->serverfd, (struct sockaddr *) NULL, NULL);
            if (client->clientfd < 0) {
                  std::cerr << "accept error\n";
                  continue;
            }
        // adding client to list
        client->clientList.push_back(client->clientfd);
        std::cout << "new client connected\n";
        std::cout << "new connection, socket fd is " << client->clientfd << ", ip is: " << inet_ntoa(server_addr.sin_addr) << ", port: " << ntohs(server_addr.sin_port) << "\n";
        char message[1024];
            for(int i=0;i<client->clientList.size();++i){
                sd=client->clientList[i];
                if (FD_ISSET(sd, &readfds)){
                valread=read(sd, message, 1024);
                //check if client disconnected
                if (valread==0){
                    std::cout<<"client disconnected\n";

                    getpeername(sd, (struct sockaddr*)&server_addr, (socklen_t*)&server_addr);
                    // getpeername name return the address of the client (sd)
            
                    std::cout<<"host disconnected, ip: "<<inet_ntoa(server_addr.sin_addr)<<", port: "<<ntohs(server_addr.sin_port)<<"\n";
                    close(sd);
                    /* remove the client from the list */
                    client->clientList.erase(client->clientList.begin()+i);
                }else{
                    std::cout<<"message from client: "<<message<<"\n";
                    /*
                    * handle the message in new thread
                    * so that we can listen to other client
                    * in the main thread
                    * std::thread t1(handleMessage, client, message);
                    * // detach the thread so that it can run independently
                    * t1.detach();
                    */
                }
            }
        }
    }
    delete client;
    return 0;
    }
}
