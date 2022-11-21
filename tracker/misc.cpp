#include "./misc.h"

std::string convertToString(char *data, int size)
{
    std::string res = "";
    for (int i = 0; i < size; i++)
    {
        res += data[i];
    }
    return res;
}

std::vector<std::string> tokenize(std::string input, std::string delimiter, int noOfTokens)
{
    std::vector<std::string> res;
    std::size_t tempPos;
    while (noOfTokens && (tempPos = input.find(delimiter)) != std::string::npos)
    {
        res.push_back(input.substr(0, tempPos));
        input = input.substr(tempPos + 1, input.size());
        noOfTokens--;
    }
    res.push_back(input);
    return res;
}

void handleRequest(std::string messageString, int clientSocket)
{
    std::string command = messageString.substr(0, messageString.find(":"));
    std::string data = messageString.substr(messageString.find(":") + 1, messageString.size());
    std::vector<std::string> tokens;
    std::string response;
    int port;
    bool status;
    std::cout << command << std::endl;
    if (command == SendPortCommand)
    {
        pthread_mutex_lock(&lock);
        std::string username = session[clientSocket];
        pthread_mutex_unlock(&lock);

        pthread_mutex_lock(&userLock);
        UserDirectory[username]->port = stoi(data);
        pthread_mutex_unlock(&userLock);

        std::cout << "Information about client " << username << " and port " << data << " stored!" << std::endl;
    }
    else if (command == UploadFileCommand)
    {
        // UploadFileCommand filename filesize groupname

        tokens = tokenize(data, ";", 2);
        // status = checkGroupExistence(tokens[2]);
        // if (status == false){
        //   response = FileNotFoundCode + ";Group does not exist";
        std::cout << "in upload misc.cpp";
        std::cout << tokens[0] << " " << tokens[1] << " " << tokens[2] << std::endl;

        if ((status = isAuthorized(tokens[2], clientSocket)) == false)
        {
            response = InvalidAuthCode + ";Not authorized to add files";
        }
        else
        {
            status = addFile(tokens[0], tokens[1], clientSocket);
            if (status == false)
                response = ResourceExistsCode + ";File already exists. Please choose a different name";
            else
                response = StatusOkCode + ";Successfully uploaded file";
        }
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    else if (command == DownloadFileCommand)
    {
        // DownloadFile groupid filename
        std::cout << "Inside download misc.cpp  --->";
        tokens = tokenize(data, ";", 2);
        std::cout << tokens[0] << " " << tokens[1] << " " << std::endl;

        //-> if ((status = isAuthorized(tokens[0], clientSocket)) == false){
        //-> response = InvalidAuthCode + ";Not authorized to download from this group";
        response = StatusOkCode + ";" + fetchFileConsumers(tokens[0], tokens[1]);

        /*  } else {
             //std::cout<<"hello";
             response = StatusOkCode + ";" + fetchFileConsumers(tokens[0], tokens[1]);
             // addFile(tokens[0], tokens[1], clientSocket);
          } */
        // std::string tt = "asdasdasd";
        //send(clientSocket, tt.c_str(), tt.size(), 0);
        
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    else if (command == CreateUserCommand)
    {
        tokens = tokenize(data, ";", 1);
        status = createUser(tokens[0], tokens[1]);
        if (status == false)
        {
            response = ResourceExistsCode + ";Username already exists";
        }
        else
        {
            response = StatusOkCode + ";Created User successfully";
        }
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    else if (command == LoginCommmand)
    {
        tokens = tokenize(data, ";", 1);
        status = login(tokens[0], tokens[1], clientSocket);
        if (status == false)
        {
            response = InvalidAuthCode + ";Invalid username or password";
        }
        else
        {
            pthread_mutex_lock(&lock);
            session[clientSocket] = tokens[0];
            pthread_mutex_unlock(&lock);
            response = StatusOkCode + ";Logged in successfully";
        }
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    else if (command == CreateGroupCommand)
    {
        status = createGroup(data, clientSocket);
        if (status == false)
        {
            response = ResourceExistsCode + ";Group already exists";
        }
        else
        {
            response = StatusOkCode + ";Created Group successfully";
        }
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    else if (command == JoinGroupCommand)
    {
        status = checkGroupExistence(data);
        if (status == false)
        {
            response = FileNotFoundCode + ";Group does not exist";
        }
        else
        {
            pthread_mutex_lock(&lock);
            std::string username = session[clientSocket];
            pthread_mutex_unlock(&lock);
            status = addGroupJoinRequests(data, username);
            if (status == false)
                response = ResourceExistsCode + ";You're aleady added to this group";
            else
                response = StatusOkCode + ";Successfully submitted your request";
        }
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    else if (command == RespondToJoinGroupRequestCommand)
    {
        tokens = tokenize(data, ";", 1);
        status = checkGroupExistence(tokens[0]);
        if (status == false)
        {
            response = FileNotFoundCode + ";Group does not exist";
        }
        else
        {
            status = acceptGroupJoinRequest(tokens[0], tokens[1], clientSocket);
            if (status == false)
                response = InvalidAuthCode + ";Could not add user to group " + tokens[0];
            else
                response = StatusOkCode + ";Successfuly added user " + tokens[1] + " to group " + tokens[0];
        }
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    else if (command == ListJoinGroupRequestsCommand)
    {
        status = checkGroupExistence(data);
        if (status == false)
        {
            response = FileNotFoundCode + ";Group does not exist";
        }
        else
        {
            response = listJoinGroupRequests(data, clientSocket);
            if (response == InvalidAuthCode)
                response = InvalidAuthCode + ";You do not have permissions for this operation";
            else
                response = StatusOkCode + ";" + response;
        }
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    else if (command == ListAllGroupsCommand)
    {
        response = StatusOkCode + ";" + listAllGroups();
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    else if (command == ListAllFiles)
    {
        status = checkGroupExistence(data);
        if (status == false)
        {
            response = FileNotFoundCode + ";Group does not exist";
        }
        else
        {
            response = StatusOkCode + ";" + listFilesInGroup(data);
        }

        send(clientSocket, response.c_str(), response.size(), 0);
    }
    else if (command == LeaveGroupCommand)
    {
        if ((status = checkGroupExistence(data)) == false)
        {
            response = FileNotFoundCode + ";Group does not exist";
        }
        else if ((status = isAuthorized(data, clientSocket)) == false)
        {
            response = InvalidAuthCode + ";Not authorized";
        }
        else
        {
            removeUserFromGroup(data, clientSocket);
            response = StatusOkCode + ";";
        }
        send(clientSocket, response.c_str(), response.size(), 0);
    }
    else
    {
        std::cout << "Client data is " << messageString << std::endl;
    }
}

void createServerSocket(int *serverSocket, int port)
{
    *serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in *server_address = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));
    inet_pton(AF_INET, address, &server_address->sin_addr);
    server_address->sin_family = AF_INET;
    server_address->sin_port = htons(port);
    server_address->sin_addr.s_addr = INADDR_ANY;
    int server_status = bind(*serverSocket, (struct sockaddr *)server_address, sizeof(*server_address));
    printf("%d\n", server_status);
    if (server_status == -1)
    {
        printf("Error allocating port\n");
        exit(1);
    }
}

void disconnectSession(int clientSocket)
{
    printf("Peer %d disconnected!\n", clientSocket);
    pthread_mutex_lock(&lock);
    logout(session[clientSocket]);
    session.erase(session.find(clientSocket));
    pthread_mutex_unlock(&lock);
    close(clientSocket);
}

void *receiveDataFunc(void *arg)
{
    int clientSocket = *((int *)arg);
    free(arg);
    char server_data[BUFFER_SIZE] = "";
    int status, remainingData;
    std::string messageBacklog = "";
    bool isDisconnectStatus = false;
    int sepPosition;
    while (1)
    {
        status = recv(clientSocket, &server_data, sizeof(server_data), 0);
        if (status <= 0)
        {
            disconnectSession(clientSocket);
            break;
        }
        messageBacklog += convertToString(server_data, status);

        // std::cout << messageBacklog << std::endl;

        sepPosition = messageBacklog.find(":");
        std::string dataLen = messageBacklog.substr(0, messageBacklog.find(":"));
        remainingData = (int)dataLen.size() + (int)stoi(dataLen) - status;
        while (remainingData > 0)
        {
            bzero(server_data, BUFFER_SIZE);
            status = recv(clientSocket, &server_data, remainingData, 0);
            if (status <= 0)
            {
                disconnectSession(clientSocket);
                isDisconnectStatus = true;
                break;
            }
            messageBacklog += convertToString(server_data, status);
            remainingData -= status;
        }
        if (isDisconnectStatus)
            break;
        // std::cout << "Status is " << status << std::endl;
        handleRequest(messageBacklog.substr(sepPosition + 1, stoi(dataLen) - 1), clientSocket);
        bzero(server_data, BUFFER_SIZE);
        messageBacklog.erase(0, dataLen.size() + stoi(dataLen));
    }
    return NULL;
}

void *listenFunc(void *arg)
{
    int *peerSocket;
    pthread_t *receiveThread;
    std::cout << "Listening for connections" << std::endl;
    while (1)
    {
        listen(server_socket, 1);
        int client_socket = accept(server_socket, NULL, NULL);
        if (client_socket == -1)
        {
            printf("Error connecting to client!\n");
            continue;
        }
        pthread_mutex_lock(&lock);
        session[client_socket] = "";
        pthread_mutex_unlock(&lock);

        receiveThread = (pthread_t *)malloc(sizeof(pthread_t));
        printf("Connected to client %d\n", client_socket);

        peerSocket = (int *)malloc(sizeof(int));
        *peerSocket = client_socket;

        // Listen to peer messages if connection is established.
        pthread_create(receiveThread, NULL, receiveDataFunc, peerSocket);
    }
    return NULL;
}