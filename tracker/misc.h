#include "./tracker_global.h"
#include "./auth.h"
#include "./group.h"

#ifndef _TMISC
#define _TMISC

#define BUFFER_SIZE 2048

void createServerSocket(int * serverSocket, int port);
std::vector<std::string> tokenize(std::string input, std::string delimiter, int noOfTokens);
void * receiveDataFunc(void * arg);
void * listenFunc(void * arg);

#endif
