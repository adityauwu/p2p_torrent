#ifndef _GGLOBALS
#define _GGLOBALS

#define CHUNK_SIZE_GLOBAL 524288

#include <string>

extern std::string SendPortCommand;
extern std::string SendUsernameCommand;
extern std::string UploadFileCommand;
extern std::string DownloadFileCommand;
extern std::string FileNotFoundCode;
extern std::string ResourceExistsCode;
extern std::string InvalidAuthCode;
extern std::string StatusOkCode;
extern std::string CreateUserCommand;
extern std::string LoginCommmand;
extern std::string FileStreamRecv;
extern std::string FileStreamRecvEnd;
extern std::string RequestFilePeer;
extern std::string CreateGroupCommand;
extern std::string JoinGroupCommand;
extern std::string RespondToJoinGroupRequestCommand;
extern std::string ListJoinGroupRequestsCommand;
extern std::string LeaveGroupCommand;
extern std::string ListAllGroupsCommand;
extern std::string GetBitVector;
extern std::string SetBitVector;
extern std::string SetIndividualBitVector;
extern std::string ChunkHashCommand;
extern std::string ListAllFiles;
extern std::string LeaveGroup;

#endif