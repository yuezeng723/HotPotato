#include "host.h"
#include "potato.h"


int main(int argc, const char *argv[]){
    if (argc != 4){
        printf("invalid input!\n");
        exit(EXIT_FAILURE);
    }
    char *port_num = NULL;
    strcpy(port_num, argv[1]);
    int num_players = atoi(argv[2]);//total number of players
    int num_hops = atoi(argv[3]);//number of total hops
    int counter = 0;
    printf("Potato Ringmaster\nPlayer = %d\nHops = %d\n", num_players, num_hops);
    int listener = serverListen(port_num);
    int serverFD;//server's file descriptor
    int myFDs[num_players];//hold file-descriptors of clients, the index corresponding to playerID
    int FD_max = 0;
    fd_set read_set, currFDs;//read_set hold original FDs
    while(counter < num_players){
        serverFD = serverAccept(listener);
        printf("Player %d is ready to play\n", counter);
        int message[2];//message[0] is playerID, message[1] is player's total number
        message[0] = counter;
        message[1] = num_players;
        send(serverFD, message, 2*sizeof(int), 0);
        myFDs[counter] = serverFD;
        FD_SET(serverFD, &read_set);
        FD_max = max(serverFD, FD_max);
    }

    //build up player socket ring

    ///ringmaster receives playerPort passed by player and send it to neighbor
    int count = num_players;
    char playerPort[BUFFERSIZE];
    char playerServerName[BUFFERSIZE];
    while(count > 0){
        currFDs = read_set;
        select(FD_max + 1, &currFDs, NULL, NULL, NULL);
        for (int i = 0; i < (num_players-1); i++){
            if (FD_ISSET(myFDs[i], &currFDs)){
                //阻塞模式的flagRecv
                int flagRecv = setBlockingFlag(myFDs[i]);
                //阻塞状态下收到 playerPort 和 playerServerName
                recv(myFDs[i], &playerPort, BUFFERSIZE, flagRecv);
                recv(myFDs[i], &playerServerName, BUFFERSIZE, flagRecv);
                num_players --;
                //阻塞模式的flagSend
                int flagSend = setBlockingFlag(myFDs[((i+1)%num_players)]);
                //阻塞状态下发送 playerPort 和 playerServerName to playerClient
                send(myFDs[((i+1)%num_players)], &playerPort, BUFFERSIZE, flagSend);
                send(myFDs[((i+1)%num_players)], &playerServerName, BUFFERSIZE, flagSend);
            }
        }
    }

    //start playing potato
    ///initialize potato
    Potato mypotato;
    mypotato.num_hops = num_hops;
    mypotato.index = 0;
    ///first send to random player
    int randPlayerID = getRandNum(num_players);
    int randomPlayerFD = myFDs[randPlayerID];//get random player to send
    printf("Ready to start the game, sending potato to player %d\n", randPlayerID);

    int flagSend = setBlockingFlag(randomPlayerFD);
    send(randomPlayerFD, &mypotato, sizeof(mypotato), flagSend);

    ///at the end, receive potato
    int potatoBuffer[520];//512 + 1 + 1= 514
    Potato * endPotato = (Potato*)potatoBuffer;
    int flagRecv = setBlockingFlag(serverFD);
    recv(serverFD, &potatoBuffer, 520*sizeof(int), flagRecv);
    ///print playerIDs on potato
    printf("Trace of potato:\n");
    for (int i = 0; i < num_hops-2; i++){
        printf("%d,", endPotato->playerIDs[i]);
    }
    printf("%d", endPotato->playerIDs[num_hops-1]);

    //ringmaster send message to tell all player end game
    ///now, the potato's num_hops is 0, send this potato to all players
    for (int i = 0; i < num_players; i++){
        send(myFDs[i], &potatoBuffer, 520*sizeof(int), 0);//non-blocking
    }

    //close all the sockets
    for (int i = 0; i < num_players; i++){
        close(myFDs[i]);
    }
    close(listener);

    return 0;
}
