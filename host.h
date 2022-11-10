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
#define BUFFERSIZE 500


#ifndef POTATO_HOST_H
#define POTATO_HOST_H

#endif //POTATO_HOST_H
int serverListen(const char * portName);
int serverAccept(int fd);
int clientConnect(const char * portName, const char * serverName);
int max(int a, int b);
char * getPortName(int fd);
char * getServerName(int fd);
int getRandNum(int range);
int setBlockingFlag(int fd);

