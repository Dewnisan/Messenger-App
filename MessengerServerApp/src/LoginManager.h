#ifndef LOGINMANAGER_H_
#define LOGINMANAGER_H_

#include <map>
#include <string>

#include "MessengerServer.h"
#include "MThread.h"
#include "TCPMessengerProtocol.h"
#include "TCPSocket.h"

// Handles the login and registration requests
class LoginManager: public MThread {
	const char* _loginFile = "connections.txt";
	bool _running;
	std::map<std::string, TCPSocket*> _peers; // peers which didn't log in
	MessengerServer* _messengerServer;

	// Search in file for the given username and password.
	bool login(string userName, string password);

	// Write to file the new user with his details
	bool signUp(string userName, string password);

public:

	LoginManager(MessengerServer* messengerServer);
	virtual ~LoginManager();

	void run();
	void addPeer(TCPSocket* peer);
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
