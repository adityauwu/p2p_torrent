#include "./auth.h"


bool createUser(std::string username, std::string password){
    bool userExists = false;
    pthread_mutex_lock(&userLock);
    if (UserDirectory.find(username) != UserDirectory.end())
        userExists = true;
    else {
        UserDirectory[username] = new UserInfo(username, password);
    }
    pthread_mutex_unlock(&userLock); 
    return !userExists;
}


bool login(std::string username, std::string password, int sessionId){
    bool isValid = true;
    pthread_mutex_lock(&userLock);
    if (UserDirectory.find(username) == UserDirectory.end())
        isValid = false;
    else {
        UserInfo * uInfo = UserDirectory[username];
        if (uInfo->password != password)
            isValid = false;
        else
            uInfo->currentSessionId = sessionId;
    }

    pthread_mutex_unlock(&userLock);

    return isValid;
}

void logout(std::string username){
    pthread_mutex_lock(&userLock);
    if (UserDirectory.find(username) != UserDirectory.end()){
        UserInfo * uInfo = UserDirectory[username];
        uInfo->currentSessionId = -1;
        uInfo->port = -1;
    }
    pthread_mutex_unlock(&userLock);
}