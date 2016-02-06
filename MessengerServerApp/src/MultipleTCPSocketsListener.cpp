

#include "MultipleTCPSocketsListener.h"

using namespace std;



void MultipleTCPSocketsListener::addSocket(TCPSocket* socket){
	sockets.push_back(socket);
}



void MultipleTCPSocketsListener::addSockets(vector<TCPSocket*> socketVec){
	sockets = socketVec;
}

void MultipleTCPSocketsListener::addSockets(map<string,TCPSocket*> socketMap)
{
	std::map<string,TCPSocket*>::iterator iter = socketMap.begin();
	std::map<string,TCPSocket*>::iterator enditer = socketMap.end();

	while (iter != enditer)
	{
		sockets.push_back(iter->second);
		iter++;
	}
}



TCPSocket* MultipleTCPSocketsListener::listenToSocket(int timeout){
	struct timeval tv = {timeout, 0};
	tSocketsContainer::iterator iter = sockets.begin();
	tSocketsContainer::iterator endIter = sockets.end();
	fd_set fdset;
	FD_ZERO(&fdset);
	int highfd = 0;
	//fill the set with file descriptors
	for (;iter != endIter;iter++) {
		highfd++;
		FD_SET((*iter)->getSocketFid(), &fdset);
		//if ((*iter)->getSocketFid()>highfd) highfd = (*iter)->getSocketFid();
	}

	//perform the select
	int returned;
	if (timeout>0){
		returned = select(sizeof(fdset)*8, &fdset, NULL, NULL, &tv);
	}else{
		returned = select(sizeof(fdset)*8, &fdset, NULL, NULL, NULL);
	}
	if (returned) {
		for (int i = 0; i < highfd; i++) {
			TCPSocket* tmpSocket = sockets[i];
			if (FD_ISSET(tmpSocket->getSocketFid(), &fdset)) {
				return tmpSocket;
			}
		}
	}
	return NULL;
}

