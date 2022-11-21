#include "./tracker_global.h"

#ifndef _TAUTH
#define _TAUTH

bool createUser(std::string username, std::string password);
bool login(std::string username, std::string passsword, int sessionId);
void logout(std::string username);

#endif