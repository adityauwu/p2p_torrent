#include "./peer_global.h"


RequestType::RequestType(){}

RequestType::RequestType(std::string message, int clientSocket, int messageSize){
    this->message = message;
    this->clientSocket = clientSocket;
    this->messageSize = messageSize;
}

FileType::FileType(std::string fileName, std::string filePath, int fileSize){
    this->fileName = fileName;
    this->filePath = filePath;
    this->fileSize = fileSize;
    this->noOfChunks = ceil(this->fileSize * 1.0 / CHUNK_SIZE_GLOBAL);
}

FileDownloadLockType::FileDownloadLockType(){
    this->FileAccessCount = 0;
    this->FileAccessMutex = PTHREAD_MUTEX_INITIALIZER;
    sem_init(&(this->FileAccessSemaphore), 0, 1);
}

BitVectorType::BitVectorType(int bit, int chunkSize, int bytesReceived, std::string hash){
    this->bit = bit;
    this->chunkSize = chunkSize;
    this->bytesReceived = bytesReceived;
    this->hash = hash;
}

ChunkRequestType::ChunkRequestType(std::string fileName, int chunkNo){
    this->fileName = fileName;
    this->chunkNo = chunkNo;
}

void FileType::setAsSeeder(std::string user){
    if (this->bitVector[user] == NULL){
        this->bitVector[user] = new std::vector<BitVectorType *>(this->noOfChunks, new BitVectorType(1, CHUNK_SIZE_GLOBAL, CHUNK_SIZE_GLOBAL, ""));
    } else {
        this->bitVector[user]->assign(this->noOfChunks, new BitVectorType(1, CHUNK_SIZE_GLOBAL, CHUNK_SIZE_GLOBAL, ""));
    }
    if (this->fileSize % CHUNK_SIZE_GLOBAL != 0){
        this->bitVector[user]->back()->chunkSize = (this->fileSize % CHUNK_SIZE_GLOBAL);

        this->bitVector[user]->back()->bytesReceived =  this->bitVector[user]->back()->chunkSize;
    }

}

void FileType::pushToChunkRequest(std::string fileName, int chunkNo){
    pthread_mutex_lock(&chunkRequestLock);
    chunkRequestQueue.push(new ChunkRequestType(fileName, chunkNo));
    pthread_cond_signal(&chunkRequestConditionVar);
    pthread_mutex_unlock(&chunkRequestLock);
}

void FileType::initializeBitVector(std::string user){
    this->bitVector[user] = new std::vector<BitVectorType *>();
    for (int i = 0; i < this->noOfChunks; i++){
        this->bitVector[user]->push_back(new BitVectorType(0, CHUNK_SIZE_GLOBAL, 0, ""));
    }
    if (this->fileSize % CHUNK_SIZE_GLOBAL != 0)
        this->bitVector[user]->back()->chunkSize = (this->fileSize % CHUNK_SIZE_GLOBAL);
}


void FileType::setParticularBitPosition(std::string user, int bitVal, int chunkPos){
    if (this->bitVector[user] == NULL)
        this->initializeBitVector(user);
    this->bitVector[user]->at(chunkPos)->bit = bitVal;
    if (bitVal == 1){
        this->bitVector[user]->at(chunkPos)->bytesReceived = this->bitVector[user]->at(chunkPos)->chunkSize;
    }
}

void FileType::setBitvectorFromString(std::string user, std::string bitVectorString){
    std::vector<int> bitVectorInt = splitString(bitVectorString, ',');
    std::cout << bitVectorString << std::endl;
    this->bitVector[user] = new std::vector<BitVectorType *>();
    int fileSizeTemp;
    for (int i = 0; i < bitVectorInt.size(); i++){
        fileSizeTemp = CHUNK_SIZE_GLOBAL;
        if (i == this->noOfChunks - 1 && this->fileSize % CHUNK_SIZE_GLOBAL != 0){
            fileSizeTemp = this->fileSize % CHUNK_SIZE_GLOBAL;
        }
        writerLock(&FileMapSemaphore);
        this->bitVector[user]->push_back(new BitVectorType(bitVectorInt[i], fileSizeTemp, 0, ""));
        if (bitVectorInt[i] == 1){
            this->bitVector[user]->back()->bytesReceived =  this->bitVector[user]->back()->chunkSize;
            chunkPossessionMap[i].insert(user);
        }
        writerUnlock(&FileMapSemaphore);
    }

    std::vector<int> chunksInt(this->noOfChunks, 0);
    for (int i = 0; i < noOfChunks; i++){
        chunksInt[i] = i;
    }

    random_shuffle(chunksInt.begin(), chunksInt.end());
    for (auto a: chunksInt){
        readerLock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);
        if (this->bitVector[user]->at(a)->bit == 1)
            pushToChunkRequest(this->fileName, a);
        readerUnlock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);
    }
}

std::string FileType::getStringFromBitVector(std::string user){
    std::string res = "";
    if (this->bitVector.find(user) == this->bitVector.end()){
        std::cout << "User not found!" << std::endl;
    }
    std::vector<BitVectorType *> * bitvec = this->bitVector[user];
    for (auto a: *bitvec){
        res += std::to_string(a->bit) + ",";
    }
    if (bitvec->size() > 0)
        res.pop_back();
    return res;
}


UserInfo::UserInfo(std::string username){
    this->username = username;
    this->currentSessionId = -1;
    this->port = -1;
}

void UserInfo::setCurrentSessionId(int sessionId){
    this->currentSessionId = sessionId;
}

void UserInfo::setPort(int port){
    this->port = port;
}


std::vector<int> FileType::splitString(std::string input, char delimiter){
    std::vector<int> res;
    std::stringstream ss(input);
    int temp;
    while (ss >> temp){
        res.push_back(temp);
        if (ss.peek() == delimiter){
            ss.ignore();
        }
    }
    return res;
    
}

void FileType::startDownload(){
    std::vector<int> chunksInt(this->noOfChunks, 0);
    for (int i = 0; i < noOfChunks; i++){
        chunksInt[i] = i;
    }
    random_shuffle(chunksInt.begin(), chunksInt.end());
    for (auto a: chunksInt){
        pushToChunkRequest(this->fileName, a);
    }
}


void writerLock(sem_t * w){
    sem_wait(w);
}

void writerUnlock(sem_t * w){
    sem_post(w);
}

void writerLock(FileDownloadLockType * w){
    sem_wait(&(w->FileAccessSemaphore));
}

void writerUnlock(FileDownloadLockType * w){
    sem_post(&(w->FileAccessSemaphore));
}

void readerLock(int * numreader, sem_t * writer, pthread_mutex_t * mutex){
    pthread_mutex_lock(mutex);
    *numreader = *numreader + 1;

    if (*numreader == 1){
        sem_wait(writer);
    }

    pthread_mutex_unlock(mutex);
}

void readerUnlock(int * numreader, sem_t * writer, pthread_mutex_t * mutex){
    pthread_mutex_lock(mutex);
    *numreader = *numreader - 1;

    if (*numreader == 0){
        sem_post(writer);
    }

    pthread_mutex_unlock(mutex);
}

void readerLock(FileDownloadLockType * w){
    pthread_mutex_lock(&(w->FileAccessMutex));
    w->FileAccessCount += 1;

    if (w->FileAccessCount == 1){
        sem_wait(&(w->FileAccessSemaphore));
    }

    pthread_mutex_unlock(&(w->FileAccessMutex));
}

void readerUnlock(FileDownloadLockType * w){
    pthread_mutex_lock(&(w->FileAccessMutex));
    w->FileAccessCount -= 1;

    if (w->FileAccessCount == 1){
        sem_post(&(w->FileAccessSemaphore));
    }

    pthread_mutex_unlock(&(w->FileAccessMutex));
}


int tracker_socket;
int peer_socket;
int tracker_port;
int peer_port;
char address[20] = "0.0.0.0";
bool isLoggedIn = false;
std::string loggedInUser;


pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t requestLock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t requestConditionVar = PTHREAD_COND_INITIALIZER;
std::map <int, std::string> session;
std::map <std::string, FileType *> FileMap;
pthread_t request_thread_pool[THREAD_POOL_SIZE];
std::queue<RequestType *> requestQueue;


pthread_mutex_t FileDownloadMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t FileDownloadSemaphore;
int FileDownloadCount;
std::map <std::string, FileDownloadLockType *> FileDownloadLockMap;

pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;

pthread_mutex_t FileMapMutex = PTHREAD_MUTEX_INITIALIZER;
sem_t FileMapSemaphore;
int FileMapCount = 0;

std::map<std::string, UserInfo *> UserDirectory;

std::queue<ChunkRequestType *> chunkRequestQueue;

pthread_cond_t chunkRequestConditionVar = PTHREAD_COND_INITIALIZER;
pthread_mutex_t chunkRequestLock = PTHREAD_MUTEX_INITIALIZER;