#include "./tracker_global.h"

#ifndef _TGROUP
#define _TGROUP

bool createGroup(std::string groupName, int clientSocket);
bool checkGroupExistence(std::string groupName);
bool addGroupJoinRequests(std::string groupName, std::string username);
std::string listJoinGroupRequests(std::string groupName, int clientSocket);
int getGroupAdminSockId(std::string groupName);
bool acceptGroupJoinRequest(std::string groupName, std::string username, int clientSocket);
std::string listAllGroups();
bool isAuthorized(std::string groupName, int clientSocket);
bool addFileToGroup(std::string groupName, std::string fileName,std::string fileSize,int clientSocket);
bool checkIfFileExistsInGroup(std::string groupName, std::string fileName);
std::string fetchFileConsumers(std::string groupName, std::string fileName);
void addUserToFileGroup(std::string groupName, std::string fileName, int clientSocket);
std::string listFilesInGroup(std::string groupName);
void removeUserFromGroup(std::string groupName, int clientSocket);
bool addFile(std::string fileName,std::string fileSize,int clientSocket);
bool checkIfFileExists(std::string fileName);
#endif