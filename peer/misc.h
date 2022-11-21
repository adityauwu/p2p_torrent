#include "./peer_global.h"
#include "./fileFunc.h"

#ifndef _PMISC
#define _PMISC

int connectPeer(int * clientSocket, int port);
void createServerSocket(int * serverSocket, int port);
void * receiveDataFunc(void * arg);
void * handleRequestThread(void * data);
void * chunkRequestThread(void * data);
void * listenFunc(void * arg);

#endif