#include<limits>
#include <ios> 
#include "./peer.h"


#define MAXFILEPATHSIZE 1024

using namespace std;

void showMenu(){
    cout << "1: Create user" << endl;
    cout << "2: Login user" << endl;
     cout << "10: Upload file" << endl;
    cout << "11: Download file" << endl;
    cout << "12: Logout" << endl;
}

int main(int argc, char ** argv){

    ifstream in;
    
    pthread_t peerThread, crThread;
    std::string ipString, portString,username, password, baseFilenameString, totalMessage, fileSize, groupName, filePathString, tempUser;
    int option;
    int * peerSocketConnect;
    char message[BUFFER_SIZE] = "";
    char recvBuffer[BUFFER_SIZE] = "";
    char * baseFileName;
    char filePath[MAXFILEPATHSIZE] = "";
    pthread_t * receiveThread;
    int tempSocket;
    vector<string> tokens, tokens2;
    FILE * fp;
    int fp_oldOpen;

    sem_init(&FileDownloadSemaphore, 0, 1);
    sem_init(&FileMapSemaphore, 0, 1);

    if (argc < 3){
        cout << "Usage: ./main peer_port_no tracker_info_file" << endl;
        exit(1);
    }

    in.open(argv[2]);
    if (in.fail()){
        cout << "Could not locate tracker file" << endl;
        exit(1);
    }
    
    in >> ipString >> portString;

    in.close();
    tracker_port = stoi(portString);
    peer_port = atoi(argv[1]);


    // Initialize thread pool
    for (int i = 0; i < THREAD_POOL_SIZE; i++){
        pthread_create(&request_thread_pool[i], NULL, handleRequestThread, NULL);
    }

    int status = connectPeer(&tracker_socket, tracker_port);
    if (status == -1){
        cout << "Unable to connect to tracker" << endl;
        exit(1);
    }
    // Create a server socket that this peer listens to

    createServerSocket(&peer_socket, peer_port);

    pthread_create(&peerThread, NULL, listenFunc, NULL);
    
    pthread_create(&crThread, NULL, chunkRequestThread, NULL);

    showMenu();
    cout << "Enter an option" << endl;
    cin >> option;
    cin.ignore(numeric_limits<streamsize>::max(),'\n');

    // Messages are sent in the following format to tracker/other peers
    // DataSize(including command):Command:Parameters/Data(separated by ;)
    while(option != 12){
        bzero(message, BUFFER_SIZE);
        bzero(recvBuffer, BUFFER_SIZE);
        switch(option){
            case 1:
                totalMessage = ":" + CreateUserCommand + ":";
                // strcpy(message, (CreateUserCommand + ":").c_str());
                cout << "Enter username and password separated by space" << endl;
                cin >> username >> password;
                totalMessage += username + ";" + password;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;

                break;

            case 2:
                totalMessage = ":" + LoginCommmand + ":";
                cout << "Enter username and password separated by space" << endl;
                cin >> username >> password;
                totalMessage += username + ";" + password;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                if (tokens[0] == InvalidAuthCode){
                    cout << "Invalid username or password" << endl;
                    break;
                } else {
                    postLogin(argv[1], tracker_socket);
                    pthread_mutex_lock(&lock);
                    isLoggedIn = true;
                    loggedInUser = username;
                    pthread_mutex_unlock(&lock);
                }
                break;
            case 3:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                totalMessage = ":" + CreateGroupCommand + ":";
                cout << "Enter group name to be created" << endl;
                cin >> groupName;
                totalMessage += groupName;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;
                break;
            case 4:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                totalMessage = ":" + JoinGroupCommand + ":";
                cout << "Enter group name to join" << endl;
                cin >> groupName;
                totalMessage += groupName;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;
                break;

            case 5:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                totalMessage = ":" + LeaveGroupCommand + ":";
                cout << "Enter group name to leave" << endl;
                cin >> groupName;
                totalMessage += groupName;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;
                break;
            
            case 6:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                totalMessage = ":" +ListJoinGroupRequestsCommand + ":";
                cout << "Enter group name to list join requests" << endl;
                cin >> groupName;
                totalMessage += groupName;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;
                break;
            
            case 7:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                totalMessage = ":" +RespondToJoinGroupRequestCommand + ":";
                cout << "Enter group name, user name whose request you want to approve" << endl;
                cin >> groupName >> username;
                totalMessage += groupName + ";" + username;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;
                break;

            case 8:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                totalMessage = ":" + ListAllGroupsCommand + ":";
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;
                break;

            case 9:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                cout << "Enter groupname" << endl;
                cin >> groupName;
                totalMessage = ":" + ListAllFiles + ":" + groupName;
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                strcpy(message, totalMessage.c_str());
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                tokens = tokenize(recvBuffer, ";", 1);
                cout << tokens[1] << endl;    

            case 10:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                std::cout << "Enter file path" << endl;
                // fgets(filePath, MAXFILEPATHSIZE, stdin);

                //cin >> filePathString >> groupName; 
                 filePathString = "/home/aditya/Downloads/p2ptest.pdf";
                 groupName = "g1";

                fp = fopen(filePathString.c_str(), "r");
                if (fp == NULL){
                    cout << "File unavailable" << endl;
                    break;
                }

                fseek(fp, 0, SEEK_END);
                fileSize = to_string(ftell(fp));
                fclose(fp);


                baseFilenameString = basename(filePathString.c_str());

                // Messagesize:Command:Filename;FileSize;Groupname

                totalMessage = ":" + UploadFileCommand + ":" + baseFilenameString + ";" + fileSize + ";" + groupName;
                
                totalMessage = to_string(totalMessage.size()) + totalMessage;
                
                strcpy(message, totalMessage.c_str());
                //cout<<message<<endl;
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);
                //cout<<"Here after uploading files"<<endl;
                tokens = tokenize(recvBuffer, ";", 1);
                
                cout << tokens[1] << endl;
                //if (tokens[0] != StatusOkCode){
                //    break;
               // }

                writerLock(&FileMapSemaphore);
                    FileMap[baseFilenameString] = new FileType(baseFilenameString, filePathString, stoi(fileSize));
                    FileMap[baseFilenameString]->setAsSeeder(loggedInUser);
                writerUnlock(&FileMapSemaphore);

                writerLock(&FileDownloadSemaphore);
                if (FileDownloadLockMap.find(baseFilenameString) == FileDownloadLockMap.end())
                    FileDownloadLockMap[baseFilenameString] = new FileDownloadLockType();
                writerUnlock(&FileDownloadSemaphore);

                break;
            case 11:
                if (!checkLogin()){
                    cout << "User not logged in!" << endl;
                    break;
                }
                
                std::cout << "<-----------Inside the download thread------>" << endl;
                
                //cin >> baseFilenameString >> filePathString >> groupName;

                // baseFilenameString = "sample-2mb-text-file.txt";
                filePathString = "/home/aditya/Downloads/p2ptest.pdf";
                //filePathString = filePathString + "/" + baseFilenameString;
                 groupName = "g1";
                baseFilenameString = basename(filePathString.c_str());
                //fileSize = to_string(ftell(fp));

               // totalMessage = ":" + DownloadFileCommand + ":" + baseFilenameString + ";" + fileSize + ";" + groupName;
                
                //totalMessage = to_string(totalMessage.size()) + totalMessage;
                totalMessage = ":" + DownloadFileCommand + ":" + groupName + ";" + baseFilenameString;
                totalMessage = to_string(totalMessage.size()) + totalMessage;

                strcpy(message, totalMessage.c_str());
                //cout<<"IN download"<<endl;
               
               
                send(tracker_socket, message, strlen(message), 0);
                recv(tracker_socket, &recvBuffer, sizeof(recvBuffer), 0);

                tokens = tokenize(recvBuffer, ";", 2);


                if (tokens[0] != StatusOkCode){
                    cout << tokens[1] << endl;
                    break;
                }

                fileSize = tokens[1];

                cout << "file size to be downloaded is-->"<<tokens[1]<< endl;

                writerLock(&FileMapSemaphore);
                    if (FileMap.find(baseFilenameString) == FileMap.end()){
                        FileMap[baseFilenameString] = new FileType(baseFilenameString, filePathString, stoi(fileSize));
                        FileMap[baseFilenameString]->initializeBitVector(loggedInUser);
                    }
                writerUnlock(&FileMapSemaphore);

                
                writerLock(&FileDownloadSemaphore);
                    if (FileDownloadLockMap.find(baseFilenameString) == FileDownloadLockMap.end()){
                        FileDownloadLockMap[baseFilenameString] = new FileDownloadLockType();
                        writerLock(FileDownloadLockMap[baseFilenameString]);
                            fp_oldOpen = open(filePathString.c_str(), O_WRONLY | O_CREAT, 0766);
                            fallocate(fp_oldOpen, 0, 0, stoi(tokens[1]));
                            close(fp_oldOpen);
                        writerUnlock(FileDownloadLockMap[baseFilenameString]);
                    }
                        
                writerUnlock(&FileDownloadSemaphore);

               
                tokens = tokenize(tokens[2], ";", -2);
               
                pthread_mutex_lock(&userLock);
                    for (auto token: tokens){
                        cout << token << endl;
                        tokens2 = tokenize(token, ",", 1);
                        if (tokens2[0] == loggedInUser)
                            continue;
                        if (UserDirectory.find(tokens2[0]) == UserDirectory.end()){
                            UserDirectory[tokens2[0]] = new UserInfo(tokens2[0]);
                        }
                        if (UserDirectory[tokens2[0]]->currentSessionId == -1){
                            status = connectPeer(&tempSocket, stoi(tokens2[1]));
                            if (status == -1)
                                continue;
                            UserDirectory[tokens2[0]]->currentSessionId = tempSocket;
                            UserDirectory[tokens2[0]]->port = stoi(tokens2[1]);

                            pthread_mutex_lock(&lock);
                                session[tempSocket] = tokens2[0];
                            pthread_mutex_unlock(&lock);

                            peerSocketConnect = (int *)(malloc(sizeof(int)));
                            *peerSocketConnect = UserDirectory[tokens2[0]]->currentSessionId;
                            receiveThread = (pthread_t *)malloc(sizeof(pthread_t));
                            pthread_create(receiveThread, NULL, receiveDataFunc, peerSocketConnect);

                            totalMessage = ":" + SendUsernameCommand + ":" + loggedInUser;
                            totalMessage = to_string(totalMessage.size()) + totalMessage;
                            send(UserDirectory[tokens2[0]]->currentSessionId, totalMessage.c_str(), totalMessage.size(), 0);
                            cout<<"On line number 382 on up of getbitvector"<<endl;
                            totalMessage = ":" + GetBitVector + ":" + baseFilenameString;
                            totalMessage = to_string(totalMessage.size()) + totalMessage;
                            send(UserDirectory[tokens2[0]]->currentSessionId, totalMessage.c_str(), totalMessage.size(), 0);
                        }
                    }
                pthread_mutex_unlock(&userLock);

                break;

            default:
                cout << "Invalid command" << endl;
                break;
        }
        showMenu();
        cout << "Enter an option" << endl;
        cin >> option;
        cin.ignore(numeric_limits<streamsize>::max(),'\n');
    }

    close(peer_socket);

    return 0;
}


bool checkLogin(){
    bool isLoggedInLocal;
    pthread_mutex_lock(&lock);
    isLoggedInLocal = isLoggedIn;
    pthread_mutex_unlock(&lock);
    return isLoggedInLocal;   
}

void postLogin(std::string port, int trackerSocket){
    // Send the port that this peer listens to
    std::string message = ":" + SendPortCommand + ":" + port;
    message = to_string(message.size()) + message;

    int status = send(trackerSocket, message.c_str(), message.size(), 0);
}