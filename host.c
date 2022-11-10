#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/fcntl.h>
#define BACKLOG 250



/**
 * Given portName string and build up listen socket
 * @param portName
 * @return file-descriptor of listen socket
 */
int serverListen(const char * portName){
    struct addrinfo hints;
    struct addrinfo *serverInfo;//pointing to filled info
    struct addrinfo *p;//temp pointer used to traverse linked list
    int yes = 1;
    int listener; //listener file-descriptor for any connection
    //fill in address-info
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;//IPv4
    hints.ai_socktype = SOCK_STREAM;//TCP stream
    hints.ai_flags = AI_PASSIVE;//let system fill in my IP automatically
    if (getaddrinfo(NULL, portName, &hints, &serverInfo) != 0) {
        perror("getaddrinfo error: can't get address information!");
        exit(EXIT_FAILURE);
    }
    //create and bind socket
    for(p = serverInfo; p != NULL; p = p->ai_next) {
        //create socket
        if ((listener = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket error: can't create socket in server!");
            continue;
        }
        if (setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt error: cant set socket!");
            exit(EXIT_FAILURE);
        }
        //bind socket
        if (bind(listener, p->ai_addr, p->ai_addrlen) == -1){
            close(listener);
            perror("bind error: can't bind socket to port!");
            continue;
        }
        break;
    }
    freeaddrinfo(serverInfo);
    if (p == NULL){
        perror("bind error: can't bind socket to port!");
        exit(EXIT_FAILURE);
    }
    //listen socket
    if(listen(listener, BACKLOG) == -1){
        perror("lister error: can't listen socket to client!");
        exit(EXIT_FAILURE);
    }
    return listener;
}

int serverAccept(int fd){
    struct sockaddr_storage theirAddr;
    socklen_t addrSize = sizeof theirAddr;
    int serverFD = accept(fd, (struct sockaddr * )&theirAddr, &addrSize);
    return serverFD;
}

/**
 * Build up client and connect to given server
 * @param portName string portName eg:"3046"
 * @param clientName string serverName eg"yz723@dku-vcm-1505.vm.duke.edu"
 * @return the read and write file descriptor for client
 */
int clientConnect(const char * portName, const char * serverName){
    struct addrinfo hints;
    struct addrinfo *serverInfo;//pointing to filled info
    struct addrinfo *p;//temp pointer used to traverse linked list
    int yes = 1;
    int clientFD; //client's file-descriptor corresponding to server's listener
    //fill in address-info
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;//IPv4
    hints.ai_socktype = SOCK_STREAM;//TCP stream

    if (getaddrinfo(serverName, portName, &hints, &serverInfo) != 0) {
        perror("getaddrinfo error: can't get address information!");
        exit(EXIT_FAILURE);
    }
    //create and connect socket
    for(p = serverInfo; p != NULL; p = p->ai_next) {
        //create socket
        if ((clientFD = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("socket error: can't create socket in server!");
            continue;
        }
        if (connect(clientFD, p->ai_addr, p->ai_addrlen) == -1) {
            perror("connect error: client can't connect!");
            exit(EXIT_FAILURE);
        }
        break;
    }
    freeaddrinfo(serverInfo);
    if (p == NULL){
        perror("connect error: client can't connect!");
        exit(EXIT_FAILURE);
    }
    return clientFD;
}

char * getPortName(int fd){
    socklen_t len;
    struct sockaddr_storage addr;
    //char ipstr[INET6_ADDRSTRLEN];
    int port;
    len = sizeof addr;
    getpeername(fd, (struct sockaddr*)&addr, &len);
    // deal with both IPv4 and IPv6:
    if (addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
        port = ntohs(s->sin_port);
        //inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
    } else { // AF_INET6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
        port = ntohs(s->sin6_port);
        //inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
    }
    char result[10];
    sprintf(result, "%ld", port);
    return result;
}

char * getServerName(int fd){
    socklen_t len;
    struct sockaddr_storage addr;
    char ipstr[INET6_ADDRSTRLEN];
    //int port;
    len = sizeof addr;
    getpeername(fd, (struct sockaddr*)&addr, &len);
    // deal with both IPv4 and IPv6:
    if (addr.ss_family == AF_INET) {
        struct sockaddr_in *s = (struct sockaddr_in *)&addr;
        //port = ntohs(s->sin_port);
        inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
    } else { // AF_INET6
        struct sockaddr_in6 *s = (struct sockaddr_in6 *)&addr;
        //port = ntohs(s->sin6_port);
        inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
    }
    return ipstr;
}

int max (int a, int b) {
    if (a>b)
        return a;
    else
        return b;
}

/**
 * get random number [0, range]
 * @param range max number of possible random number
 * @return a random number from 0 to range
 */
int getRandNum(int range){
    srand( (unsigned int) time(NULL) + range);
    int random = rand()%range;
    return random;

}

int setBlockingFlag(int fd){
    int flag = fcntl(fd,F_GETFL,0);
    fcntl(fd,F_SETFL,flag&~O_NONBLOCK);
    return flag;
}