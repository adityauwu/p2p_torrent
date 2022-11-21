#include "./misc.h"

int connectPeer(int * clientSocket, int port){
    *clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in  * server_address = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(port);
    server_address->sin_addr.s_addr = INADDR_ANY;
    int conn_status = connect(*clientSocket, (struct sockaddr *) server_address, sizeof(*server_address));
    return conn_status;
}

void handleRequest(std::string messageString, int clientSocket){
    // std::cout << messageString << std::endl;
    std::string command = messageString.substr(0, messageString.find(":"));
    std::string data = messageString.substr(messageString.find(":") + 1, messageString.size());
    std::vector<std::string> tokens;
    std::string response;

    if (command == FileStreamRecv){
        std::string fileName = data;
        downloadFile(fileName, clientSocket);
    } else if (command == RequestFilePeer){
        tokens = tokenize(data, ";", 1);

        //uploadFile(filename, chunkNo, clientSocket)
        uploadFile(tokens[0], stoi(tokens[1]),clientSocket);
    } else if (command == SendUsernameCommand){
        pthread_mutex_lock(&lock);
        session[clientSocket] = data;
        pthread_mutex_unlock(&lock);
        
        pthread_mutex_lock(&userLock);

        if (UserDirectory.find(data) == UserDirectory.end())
            UserDirectory[data] = new UserInfo(data);
        UserDirectory[data]->currentSessionId = clientSocket;

        pthread_mutex_unlock(&userLock);
        
        std::cout << "Connected to user " << data << std::endl;
    }else if (command == GetBitVector){
        // tokens = tokenize(data, ";", 1);

        readerLock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);
            response = ":" + SetBitVector + ":" + loggedInUser + ";"  + data + ";" + FileMap[data]->getStringFromBitVector(loggedInUser);
        readerUnlock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);

        response = std::to_string(response.size()) + response;
        send(clientSocket, response.c_str(), response.size(), 0);
    }else if (command == SetBitVector){
        tokens = tokenize(data, ";", 2);
        // std::cout << "Token size is " << tokens.size() << std::endl;
        // Note: Semaphore is being taken care of inside the function
        FileMap[tokens[1]]->setBitvectorFromString(tokens[0], tokens[2]);
        // std::cout << "Successfully set tokens!" << std::endl;
    }else if (command == SetIndividualBitVector){
        tokens = tokenize(data, ";", 3);        
        readerLock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);
            FileMap[tokens[1]]->setParticularBitPosition(tokens[0], stoi(tokens[2]), stoi(tokens[3]));
            FileMap[tokens[1]]->chunkPossessionMap[stoi(tokens[3])].insert(tokens[0]);
        readerUnlock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);
    } else {
        std::cout << "Data not recognized!" << std::endl;
    }
    // fflush(stdin);
}

void * chunkRequestThread(void * data){
    ChunkRequestType * crType;
    int sessionId, status;
    std::string totalMessage;
    bool isDownload = true;
    std::vector<std::string> users;
    while(true){
        users.clear();
        pthread_mutex_lock(&chunkRequestLock);
        while(chunkRequestQueue.empty()){
            pthread_cond_wait(&chunkRequestConditionVar, &chunkRequestLock);
        }
        crType = chunkRequestQueue.front();
        chunkRequestQueue.pop();
        pthread_mutex_unlock(&chunkRequestLock);

        readerLock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);
        if (FileMap[crType->fileName]->bitVector[loggedInUser]->at(crType->chunkNo)->bit == 1){
            isDownload = false;
        } else {
            for (auto a: FileMap[crType->fileName]->chunkPossessionMap[crType->chunkNo])
                users.push_back(a);
        }

        readerUnlock(&FileMapCount, &FileMapSemaphore, &FileMapMutex);

        if (!isDownload){
            delete crType;
            continue;
        }

        random_shuffle(users.begin(), users.end());

        for (auto a: users){
            // std::cout << "Sent message to user " << a << std::endl;
            pthread_mutex_lock(&userLock);
            sessionId = UserDirectory[a]->currentSessionId;
            pthread_mutex_unlock(&userLock);
            if (sessionId == -1)
                continue;
            totalMessage = ":" + RequestFilePeer + ":" + crType->fileName + ";" + std::to_string(crType->chunkNo);
            totalMessage = std::to_string(totalMessage.size()) + totalMessage;
            status = send(sessionId, totalMessage.c_str(), totalMessage.size(), 0);
            if (status != -1)
                break;
        }

        // pthread_mutex_lock(&chunkRequestLock);
        // chunkRequestQueue.push(crType);
        // pthread_mutex_unlock(&chunkRequestLock);

    }
}

void * handleRequestThread(void * data){
    RequestType * rType;
    while(true){
        pthread_mutex_lock(&requestLock);
        while (requestQueue.empty()){
            pthread_cond_wait(&requestConditionVar, &requestLock);
        }
        rType = requestQueue.front();
        requestQueue.pop();
        pthread_mutex_unlock(&requestLock);
        handleRequest(rType->message, rType->clientSocket);

        delete rType;
    }
    return NULL;
}


void createServerSocket(int * serverSocket, int port){
    *serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in  * server_address = (struct sockaddr_in  *) malloc(sizeof(struct sockaddr_in));
    inet_pton(AF_INET, address, &server_address->sin_addr);
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(port);
    server_address->sin_addr.s_addr = INADDR_ANY;
    int server_status = bind(*serverSocket, (struct sockaddr *) server_address, sizeof(*server_address));
    printf("%d\n", server_status);
    if (server_status == -1){
        printf("Error allocating port\n");
        exit(1);
    }
}


void disconnectSession(int clientSocket){
    printf("Peer %d disconnected!\n",clientSocket);
    std::string username;
    pthread_mutex_lock(&lock);
    username = session[clientSocket];
    session.erase(session.find(clientSocket));
    pthread_mutex_unlock(&lock);

    pthread_mutex_lock(&userLock);
    if (UserDirectory.find(username) != UserDirectory.end()){
        UserDirectory[username]->currentSessionId = -1;
        UserDirectory[username]->port = -1;
    }
    pthread_mutex_unlock(&userLock);
    close(clientSocket);
}

void flushBacklog (std::string &messageBacklog, int clientSocket){
    int sepPosition;
    int dataLen;
    std::string dataLenString;
    std::string finalMessage;
    // std::cout << "In flush backlog initial size is " <<  messageBacklog.size()<< std::endl;
    while(messageBacklog.size() >  0 && (sepPosition = messageBacklog.find(":")) != std::string::npos){
        sepPosition = messageBacklog.find(":");
        dataLenString = messageBacklog.substr(0, sepPosition);
        // std::cout << "DatalenString is " << dataLenString << std::endl;
        // std::cout << "Interim message backlog size is " << messageBacklog.size() << std::endl;
        // std::cout << "Subtract: " << (messageBacklog.size() - dataLen - dataLenString.size()) << std::endl;
        dataLen = stoi(dataLenString);
        if (((int)messageBacklog.size() - dataLen - (int)dataLenString.size()) < 0)
            break;

        finalMessage = messageBacklog.substr(sepPosition + 1, dataLen - 1);
        pthread_mutex_lock(&requestLock);
        requestQueue.push(new RequestType(finalMessage, clientSocket, finalMessage.size()));
        pthread_cond_signal(&requestConditionVar);
        pthread_mutex_unlock(&requestLock);

        messageBacklog.erase(0, dataLenString.size() + dataLen);
    }
//    std::cout << "Final block size is " <<  messageBacklog.size()<< std::endl;
}

void * receiveDataFunc(void * arg){
    int clientSocket = *((int *) arg);
    free(arg);
    char server_data[BUFFER_SIZE] = "";
    pthread_t * requestHandlerThread;
    int status, remainingData;
    std::string messageBacklog = "", finalMessage;
    bool isDisconnectStatus = false;
    int sepPosition;
    // std::cout << "Listening to connections for " << clientSocket << std::endl;
    while(1){
        if (messageBacklog.size() > 0)
            flushBacklog(messageBacklog, clientSocket);
        // std::cout << "Baclog block size is " <<  messageBacklog.size()<< std::endl;            
        status = recv(clientSocket, &server_data, sizeof(server_data), 0);
        
        if (status <= 0){
            disconnectSession(clientSocket);
            break;
        }
        messageBacklog += convertToString(server_data, status);
        // std::cout << "Backlog " << messageBacklog << std::endl;

        // std::cout << messageBacklog << std::endl;

        sepPosition = messageBacklog.find(":");
        std::string dataLen = messageBacklog.substr(0, sepPosition);
        // std::cout << "In receive func, datalen is " << dataLen << std::endl;
        remainingData = (int)dataLen.size() + (int)stoi(dataLen) - messageBacklog.size();
        while (remainingData > 0){
            // std::cout << "In while loop!" << std::endl;
            bzero(server_data, BUFFER_SIZE);
            status = recv(clientSocket, &server_data, remainingData, 0);
            if (status <= 0){
                disconnectSession(clientSocket);
                isDisconnectStatus = true;
                break;
            }
            messageBacklog += convertToString(server_data, status);
            // std::cout << "Received remaining data " << status << std::endl;
            remainingData -= status;
        }
        // std::cout << "Exited while loop!" << std::endl;
        if (isDisconnectStatus)
            break;
        
        finalMessage = messageBacklog.substr(sepPosition + 1, stoi(dataLen) - 1);

        pthread_mutex_lock(&requestLock);
        requestQueue.push(new RequestType(finalMessage, clientSocket, finalMessage.size()));
        pthread_cond_signal(&requestConditionVar);
        pthread_mutex_unlock(&requestLock);

        // handleRequest(messageBacklog.substr(sepPosition + 1, stoi(dataLen)), clientSocket);
        bzero(server_data, BUFFER_SIZE);
        messageBacklog.erase(0, dataLen.size() + stoi(dataLen));
    }
    return NULL;
}


void * listenFunc(void * arg){
    int * peerSocket;
    pthread_t * receiveThread;
    // std::cout << "Listening for connections" << std::endl;
    while(1){
        listen(peer_socket, 100);
        int client_socket = accept(peer_socket, NULL, NULL);
        if (client_socket == -1){
            printf("Error connecting to client!\n");
            continue;
        }
        pthread_mutex_lock(&lock);
        session[client_socket] = "";
        pthread_mutex_unlock(&lock);

        receiveThread = (pthread_t *)malloc(sizeof(pthread_t));
        // printf("Connected to client %d\n", client_socket);

        peerSocket = (int *)malloc(sizeof(int));
        *peerSocket = client_socket;

        // Listen to peer messages if connection is established.
        pthread_create(receiveThread, NULL, receiveDataFunc, peerSocket);
    }
    return NULL;
}