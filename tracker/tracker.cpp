#include "./tracker.h"

using namespace std;

int main(int argc, char ** argv){

    ifstream in;
    string ipString, portString;

    if (argc < 2){
        cout << "Usage: ./tracker tracker_file" << endl;
        exit(1);
    }

    in.open(argv[1]);
    if (in.fail()){
        cout << "Could not locate tracker file" << endl;
        exit(1);
    }

    in >> ipString >> portString;

    in.close();

    port = stoi(portString);
    createServerSocket(&server_socket, port);

    cout << "Tracker online at port " << port << endl;

    pthread_t serverThread;

    pthread_create(&serverThread, NULL, listenFunc, NULL);

    std::string command;

    // while(command != "quit"){
    //     cin >> command;
    // }

    pthread_join(serverThread, NULL);


    return 0;
}