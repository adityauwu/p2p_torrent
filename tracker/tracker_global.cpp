#include "./tracker_global.h"

FileInfo::FileInfo(){};

FileInfo::FileInfo(std::string fname, std::string fileSize){
    this->fileName = fname;
    this->fileSize = fileSize;
};

UserInfo::UserInfo(std::string username, std::string password){
    this->username = username;
    this->password = password;
}

GroupInfo::GroupInfo(std::string groupName, std::string groupOwner){
    this->groupName = groupName;
    this->groupAdmin = groupOwner;
    this->users.insert(groupName);
}


int server_socket;
int port;
char address[20] = "0.0.0.0";
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t groupLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t FileMapLock = PTHREAD_MUTEX_INITIALIZER;

std::map <int, std::string>  session;
std::map<std::string, UserInfo *> UserDirectory;
std::map<std::string, GroupInfo *> GroupDirectory;
std::map <std::string, FileInfo*> FileMap;
