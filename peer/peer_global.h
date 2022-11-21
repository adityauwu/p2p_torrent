#ifndef _PGLOBALS
#define _PGLOBALS

#include <iostream>
#include <stdlib.h>
#include <sys/socket.h>
#include <fstream>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <pthread.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <map>
#include <vector>
#include <string>
#include <set>
#include <algorithm>
#include <queue>
#include <math.h>
#include <openssl/sha.h>
#include <sstream> 
#include "./../global_commands.h"


#define BUFFER_SIZE 4096
#define CHUNK_SIZE 1024
#define COMMAND_SIZE 100
#define MAX_BACKLOG_SIZE 20000
#define THREAD_POOL_SIZE 30

typedef struct BitVectorType{
    int bit;
    int chunkSize;
    int bytesReceived;
    std::string hash;
    
    BitVectorType(int bit, int chunkSize, int bytesReceived, std::string hash);


}BitVectorType;

typedef struct FileType {
    std::string fileName;
    std::string filePath;
    int fileSize;
    int noOfChunks;
    std::map<std::string, std::vector<BitVectorType *> * > bitVector;
    std::map<int, std::set<std::string>> chunkPossessionMap;

    FileType(std::string fileName, std::string filePath, int fileSize);

    void setAsSeeder(std::string user);

    std::vector<int> splitString(std::string input, char delimiter);

    void setBitvectorFromString(std::string user, std::string bitVectorString);
    std::string getStringFromBitVector(std::string user);
    void initializeBitVector(std::string user);

    void pushToChunkRequest(std::string fileName, int chunkNo);

    void setParticularBitPosition(std::string user, int bitVal, int chunkPos);

    void startDownload();

}FileType;

struct RequestType {
    std::string message;
    int clientSocket;
    int messageSize;
    RequestType();

    RequestType(std::string message, int clientSocket, int messageSize);

};

typedef struct FileDownloadLockType {
    pthread_mutex_t FileAccessMutex;
    sem_t FileAccessSemaphore;
    int FileAccessCount;

    FileDownloadLockType();
}FileDownloadLockType;


typedef struct UserInfo {
    std::string username;
    int currentSessionId;
    int port;
    UserInfo(std::string username);

    void setCurrentSessionId(int sessionId);
    void setPort(int sessionId);

}UserInfo;

typedef struct ChunkRequestType {
    std::string fileName;
    int chunkNo;

    ChunkRequestType(std::string fileName, int chunkNo);
}ChunkRequestType;


extern int tracker_socket;
extern int peer_socket;
extern int tracker_port;
extern int peer_port;
extern bool isLoggedIn;
extern std::string loggedInUser;

extern char address[20];


extern pthread_mutex_t lock;
extern pthread_mutex_t requestLock;
extern pthread_cond_t requestConditionVar;

extern pthread_mutex_t FileDownloadMutex;
extern sem_t FileDownloadSemaphore;
extern int FileDownloadCount;
extern std::map <std::string, FileDownloadLockType *>  FileDownloadLockMap;

extern pthread_mutex_t FileMapMutex;
extern sem_t FileMapSemaphore;
extern int FileMapCount;

void writerLock(sem_t * w);
void writerUnlock(sem_t * w);
void readerLock(int * numreader, sem_t * writer, pthread_mutex_t * mutex);
void readerUnlock(int * numreader, sem_t * writer, pthread_mutex_t * mutex);

extern pthread_mutex_t userLock;


void writerLock(FileDownloadLockType * w);

void writerUnlock(FileDownloadLockType * w);

void readerLock(FileDownloadLockType * w);

void readerUnlock(FileDownloadLockType * w);

extern std::map <int, std::string> session;
extern std::map <std::string, FileType *> FileMap;
extern pthread_t request_thread_pool[THREAD_POOL_SIZE];
extern std::queue<RequestType *> requestQueue;
extern std::queue<ChunkRequestType *> chunkRequestQueue;
extern std::map<std::string, UserInfo *> UserDirectory;

extern pthread_mutex_t chunkRequestLock;
extern pthread_cond_t chunkRequestConditionVar;

#endif
