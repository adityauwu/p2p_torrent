#ifndef _TGLOBALS
#define _TGLOBALS

#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <string.h>
#include <map>
#include <vector>
#include <set>
#include <fstream>
#include "./../global_commands.h"

struct FileInfo {
    std::string fileName;
    std::string fileSize;
    std::set<std::string> consumers;
    FileInfo();
    FileInfo(std::string fname, std::string filesize);
};

typedef struct UserInfo {
    std::string username;
    std::string password;
    int currentSessionId;
    int port;
    UserInfo(std::string username, std::string password);

}UserInfo;

typedef struct GroupInfo {
    std::string groupName;
    std::string groupAdmin;
    std::map<std::string, FileInfo *> files;
    std::set<std::string> users;
    std::set<std::string> pendingRequests;
    GroupInfo(std::string groupName, std::string groupOwner);

}GroupInfo;

extern int server_socket;
extern int port;
extern char address[20];
extern pthread_mutex_t lock;
extern pthread_mutex_t userLock;
extern pthread_mutex_t groupLock;
extern pthread_mutex_t FileMapLock;

//std::map<std::string, FileInfo *> Files;
extern std::map <int, std::string> session;
extern std::map<std::string, UserInfo *> UserDirectory;
extern std::map<std::string, GroupInfo *> GroupDirectory;

extern std::map <std::string,FileInfo*> FileMap;

#endif
