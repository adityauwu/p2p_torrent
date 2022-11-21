#include "./peer_global.h"


#ifndef _PFILEFUNC
#define _PFILEFUNC


struct FileSocket {
    int clientSocket;
    int chunkNo;
    std::string fileName;
    FileSocket();
    FileSocket(int clientSocket, std::string fileName, int chunkNo);
};

void uploadFile(std::string fileName, int chunkNo,int clientSocket);
void downloadFile(std::string fileName, int clientSocket);
void substring(char * destination, char * source, int start, int len);
std::string convertToString(char * data, int size);
int find(char * str, char needle);
std::vector<std::string> tokenize(std::string input, std::string delimiter, int noOfTokens);

#endif