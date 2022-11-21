PEER_FILES = misc peer peer_global fileFunc
TRACKER_FILES = misc tracker tracker_global auth group

all: mainT mainP

mainP: mainP.o
	g++ global_commands.o $(foreach var,$(PEER_FILES),./peer/$(var).o) -o ./peer/main -pthread -lssl -lcrypto

mainT: mainT.o
	g++ global_commands.o $(foreach var,$(TRACKER_FILES),./tracker/$(var).o) -o ./tracker/main -pthread

mainP.o: global_commands.o
	$(foreach var,$(PEER_FILES),g++ -c ./peer/$(var).cpp -o ./peer/$(var).o;)

mainT.o: global_commands.o
	$(foreach var,$(TRACKER_FILES),g++ -c ./tracker/$(var).cpp -o ./tracker/$(var).o;)

global_commands.o:global_commands.cpp
	g++ -c ./global_commands.cpp -o global_commands.o

clean:
	rm ./global_commands.o
	rm ./tracker/*.o ./tracker/main
	rm ./peer/*.o ./peer/main