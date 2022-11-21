#include "./group.h"


bool checkGroupExistence(std::string groupName){
    bool doesGroupExist = false;
    pthread_mutex_lock(&groupLock);
    if (GroupDirectory.find(groupName) != GroupDirectory.end())
        doesGroupExist = true;
    pthread_mutex_unlock(&groupLock);

    return doesGroupExist;
}


int getGroupAdminSockId(std::string groupName){
    int sockId;
    std::string groupAdmin;
    pthread_mutex_lock(&groupLock);
        groupAdmin = GroupDirectory[groupName]->groupAdmin;
    pthread_mutex_unlock(&groupLock);

    pthread_mutex_lock(&userLock);
        sockId = UserDirectory[groupAdmin]->currentSessionId;
    pthread_mutex_unlock(&userLock);

    return sockId;
}

bool addGroupJoinRequests(std::string groupName, std::string username){
    bool successfullyAdded = true;
    pthread_mutex_lock(&groupLock);
        if (GroupDirectory[groupName]->users.find(username) != GroupDirectory[groupName]->users.end()){
            successfullyAdded = false;
        } else {
            GroupDirectory[groupName]->pendingRequests.insert(username);
        }
    pthread_mutex_unlock(&groupLock);
    return successfullyAdded;
}

bool createGroup(std::string groupName, int clientSocket){
    bool groupCreateSuccess = true;
    std::string username;
    pthread_mutex_lock(&groupLock);
    if (GroupDirectory.find(groupName) != GroupDirectory.end()){
        groupCreateSuccess = false;
    } else {
        pthread_mutex_lock(&lock);
        username = session[clientSocket];
        pthread_mutex_unlock(&lock);

        GroupDirectory[groupName] = new GroupInfo(groupName, username);
        GroupDirectory[groupName]->users.insert(username);
    }
    pthread_mutex_unlock(&groupLock);

    return groupCreateSuccess;
}

std::string listAllGroups(){
    std::string response = "";
    pthread_mutex_lock(&groupLock);
        for (auto a : GroupDirectory)
            response += a.first;
    pthread_mutex_unlock(&groupLock);

    return response;
}

bool isAuthorized(std::string groupName, int clientSocket){
    // bool isAuthorizedBool = true;
    // std::string username;
    // pthread_mutex_lock(&lock);
    //     username = session[clientSocket];
    // pthread_mutex_unlock(&lock);

    // pthread_mutex_lock(&groupLock);
    //     GroupInfo * ginfo = GroupDirectory[groupName];
    //     if (ginfo->users.find(username)  == ginfo->users.end())
    //         isAuthorizedBool = false;
    // pthread_mutex_unlock(&groupLock);
    // return isAuthorizedBool;
    return true;
}

std::string fetchFileConsumers(std::string groupName, std::string fileName){
    std::set<std::string> fileConsumers;
    std::string response = "";

    pthread_mutex_lock(&FileMapLock);
        //GroupInfo * ginfo = GroupDirectory[groupName];
        FileInfo*fInfo=FileMap[fileName];
        //FileInfo * fInfo = ginfo->files[fileName];
        response += fInfo->fileSize + ";";
        fileConsumers = fInfo->consumers;
        pthread_mutex_unlock(&FileMapLock);

    pthread_mutex_lock(&userLock);
    for (auto uname: fileConsumers){
        response += (uname + "," + std::to_string(UserDirectory[uname]->port) + ";");
    }
    pthread_mutex_unlock(&userLock);

    if (fileConsumers.size() > 0)
        response.pop_back();

    return response;

}

bool checkIfFileExistsInGroup(std::string groupName, std::string fileName){
    bool isExist = true;

    pthread_mutex_lock(&groupLock);
        GroupInfo * ginfo = GroupDirectory[groupName];
        if (ginfo->files.find(fileName) == ginfo->files.end())
            isExist = false;
    pthread_mutex_unlock(&groupLock);

    return isExist;
}

bool checkIfFileExists(std::string fileName){
    bool isExist = true;

    pthread_mutex_lock(&FileMapLock);
       
        if (FileMap.find(fileName) == FileMap.end())
            isExist = false;
    pthread_mutex_unlock(&FileMapLock);

    return isExist;
}


bool addFile(std::string fileName,std::string fileSize,int clientSocket){
    bool isAdded = true;
    std::string username;

    pthread_mutex_lock(&lock);
        username = session[clientSocket];
    pthread_mutex_unlock(&lock);
        
    pthread_mutex_lock(&FileMapLock);
       // GroupInfo * ginfo = GroupDirectory[groupName];
        if (FileMap.find(fileName) != FileMap.end()){
             FileMap[fileName]->consumers.insert(username);
             //isAdded = false;
        }
        else {
            FileMap[fileName] = new FileInfo(fileName, fileSize);
            FileMap[fileName]->consumers.insert(username);
        }
    pthread_mutex_unlock(&FileMapLock);

    return isAdded;


}


bool addFileToGroup(std::string groupName, std::string fileName,std::string fileSize,int clientSocket){
    bool isAdded = true;
    std::string username;

    pthread_mutex_lock(&lock);
        username = session[clientSocket];
    pthread_mutex_unlock(&lock);

    pthread_mutex_lock(&groupLock);
        GroupInfo * ginfo = GroupDirectory[groupName];
        if (ginfo->files.find(fileName) != ginfo->files.end())
            isAdded = false;
        else {
            ginfo->files[fileName] = new FileInfo(fileName, fileSize);
            ginfo->files[fileName]->consumers.insert(username);
        }
    pthread_mutex_unlock(&groupLock);

    return isAdded;
}

std::string listFilesInGroup(std::string groupName){
    std::string res = "";
    pthread_mutex_lock(&groupLock);
        GroupInfo * ginfo = GroupDirectory[groupName];
        for (auto a: ginfo->files){
            res += a.first + ";";
        }
    pthread_mutex_unlock(&groupLock);
    if (res.size() > 0){
        res.pop_back();
    }
    return res;
}

void addUserToFileGroup(std::string groupName, std::string fileName, int clientSocket){
    std::string username;
    pthread_mutex_lock(&lock);
        username = session[clientSocket];
    pthread_mutex_unlock(&lock);

    pthread_mutex_lock(&groupLock);
        GroupInfo * ginfo = GroupDirectory[groupName];
        ginfo->files[fileName]->consumers.insert(username);
    pthread_mutex_unlock(&groupLock);
}

void removeUserFromGroup(std::string groupName, int clientSocket){
    std::string username;
    pthread_mutex_lock(&lock);
        username = session[clientSocket];
    pthread_mutex_unlock(&lock);

    pthread_mutex_lock(&groupLock);
        GroupInfo * ginfo = GroupDirectory[groupName];
        for (auto a: ginfo->files){
            if (a.second->consumers.find(username) != a.second->consumers.end())
                a.second->consumers.erase(a.second->consumers.find(username));
        }
        ginfo->users.erase(ginfo->users.find(username));
    pthread_mutex_unlock(&groupLock);

}

bool acceptGroupJoinRequest(std::string groupName, std::string username, int clientSocket){
    bool isAdded = true;
    std::set<std::string>::iterator position;
    std::string requester, response;
        pthread_mutex_lock(&lock);
        requester = session[clientSocket];
    pthread_mutex_unlock(&lock);

    pthread_mutex_lock(&groupLock);
        if (GroupDirectory[groupName]->groupAdmin != requester || (position = GroupDirectory[groupName]->pendingRequests.find(username)) == GroupDirectory[groupName]->pendingRequests.end()){
            isAdded = false;
        }else {
            GroupDirectory[groupName]->users.insert(username);
            GroupDirectory[groupName]->pendingRequests.erase(position);
        }
    pthread_mutex_unlock(&groupLock);

    return isAdded;
}

std::string listJoinGroupRequests(std::string groupName, int clientSocket){
    std::string requester, response = "";
    pthread_mutex_lock(&lock);
    requester = session[clientSocket];
    pthread_mutex_unlock(&lock);

    
    pthread_mutex_lock(&groupLock);
        if (GroupDirectory[groupName]->groupAdmin != requester){
            response = InvalidAuthCode;
        }else {
            for (std::string a: GroupDirectory[groupName]->pendingRequests){
                response += (a + ";");
            }
            if (response.size() > 0){
                response.pop_back();
            }
        }
    pthread_mutex_unlock(&groupLock);

    return response;
}