#include "host.h"
#include "potato.h"

int main(int argc, const char *argv[]){
    if (argc != 3){
        printf("invalid input!\n");
        exit(EXIT_FAILURE);
    }
    char *machine_name = NULL;
    char *port_num = NULL;
    strcpy(machine_name, argv[1]);
    strcpy(port_num, argv[2]);
    int clientFD = clientConnect(port_num, machine_name);
    int playerInfo[2];//playerInfo[0] is playerD; playerInfo[1] is total player number
    recv(clientFD, playerInfo, 2*sizeof(int), 0);
    printf("Connected as player %d out of %d total players\n", playerInfo[0], playerInfo[1]);

    //build up player socket ring

    ///player build up server for index+1 player
    int playerListener = serverListen(NULL);
    ///player send its port to ringmaster
    const char * portName = getPortName(playerListener);
    const char * playerServerName = getServerName(playerListener);
    //阻塞模式的flagSend
    int flagSend = setBlockingFlag(clientFD);
    //阻塞状态下发送 playerPort 和 playerServerName
    send(clientFD, &portName, strlen(portName), flagSend);
    send(clientFD, &playerServerName, strlen(playerServerName), flagSend);
    ///player receive its neighbor's portName and serverName
    char neighborPortName[BUFFERSIZE];
    char neighborServerName[BUFFERSIZE];
    //阻塞模式下的flagRecv
    int flagRecv = setBlockingFlag(clientFD);
    int numbytes = 0;//used to add '\0', can count number of  received bytes
    numbytes = recv(clientFD, &neighborPortName, BUFFERSIZE, flagRecv);
    neighborPortName[numbytes] = '\0';//convert the buffer to string
    numbytes = recv(clientFD, &neighborServerName, BUFFERSIZE, flagRecv);
    neighborServerName[numbytes] = '\0';//convert buffer to string
    ///player build up client and connect to the server
    int playerClientFD = clientConnect(neighborPortName, neighborPortName);
    ///player as server accept other player's connection
    int playerServerFD  = serverAccept(playerListener);

    //play potato
    ///make fd_set(clientFD, playerClientFD, playerServerFD)
    fd_set playerFDs, currFDs;
    FD_SET(clientFD, &playerFDs);
    FD_SET(playerClientFD, &playerFDs);
    FD_SET(playerServerFD, &playerFDs);
    int fdmax = max(clientFD, playerClientFD);
    fdmax = max(fdmax, playerServerFD);
    int potatoBuffer[520];//512 + 1 + 1= 514
    int recvFlag, sendFlag;

    int endPoint = 1;
    while(endPoint){
        currFDs = playerFDs;
        select(fdmax+1, &currFDs, NULL, NULL, NULL);
        for (int i = 0; i < fdmax; i++){
            recvFlag = setBlockingFlag(i);
            recv(i, &potatoBuffer, 520*sizeof(int), recvFlag);//receive potato

            Potato * myPotato = potatoBuffer;
            ///check whether to end-game, if num_hops is 0, end game
            if (myPotato->num_hops == 0){
                endPoint = 0;//jump out if the while loop
                break;
            }
            myPotato->playerIDs[myPotato->index] = playerInfo[0];//add playerID on potato
            myPotato->index++;//update index
            myPotato->num_hops--;//update hops

            if (myPotato->num_hops != 0) {
                int server_or_client = getRandNum(1);
                if (server_or_client == 0) {
                    //be as server and chose to send to client((index+1)%num_players) if rand is 0
                    sendFlag = setBlockingFlag(playerServerFD);
                    send(playerServerFD, &potatoBuffer, 520 * sizeof(int), sendFlag);
                    printf("Sending potato to %d\n", ((playerInfo[0] + 1) % playerInfo[1]));
                } else {
                    //chose to send to server((index-1)%num_players) if rand is 1
                    sendFlag = setBlockingFlag(playerClientFD);
                    send(playerClientFD, &potatoBuffer, 520 * sizeof(int), sendFlag);
                    printf("Sending potato to %d\n", ((playerInfo[0] - 1) % playerInfo[1]));
                }
            }else{
                //when hop is 0, send potato to master
                printf("I'm it\n");
                sendFlag = setBlockingFlag(clientFD);//i should be clientFD
                send(clientFD, &potatoBuffer, 520 * sizeof(int), sendFlag);
                break;//don't end while loop
            }
        }
    }

    close(clientFD);
    close(playerClientFD);
    close(playerServerFD);
    close(playerListener);

    return 0;
}