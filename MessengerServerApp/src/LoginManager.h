#ifndef LOGINMANAGER_H_
#define LOGINMANAGER_H_

#include <map>
#include <string>

#include "MessengerServer.h"
#include "MThread.h"
#include "MultipleTCPSocketsListener.h"
#include "TCPMessengerProtocol.h"
#include "TCPSocket.h"

// Handles the login and registration requests
class LoginManager: public MThread {
	const char* _loginFile = "connections.txt";
	std::map<std::string, TCPSocket*> peers; // peers who didn't logged in
	bool running;
	MultipleTCPSocketsListener *listener;
	MessengerServer* _serverManager;

	bool login(string userName, string password);
	bool signUp(string userName, string password);

public:

	LoginManager(MessengerServer* _serverManager);
	void run();
	void addPeer(TCPSocket* peer);
	virtual ~LoginManager();
};

class Connection: public MThread {
	LoginManager* registration;
	TCPSocket * clientSocket;
	bool running;

public:

	Connection(LoginManager* registration);
	void run();
	virtual ~Connection();
};

#endif /* LOGINMANAGER_H_ */
