#ifndef LOGINMANAGER_H_
#define LOGINMANAGER_H_

#include <map>
#include <string>

#include "MessengerServer.h"
#include "MThread.h"
#include "TCPSocket.h"

// Handles the login and registration requests
class LoginManager: public MThread {
	bool _running;
	std::map<std::string, TCPSocket*> _peers; // peers which didn't log in
	MessengerServer* _messengerServer;

	// Search in file for the given username and password.
	bool login(string name, string password);

	// Write to file the new user with his details
	bool signUp(string name, string password);

public:

	LoginManager(MessengerServer* messengerServer);
	virtual ~LoginManager();

	void run();
	void addPeer(TCPSocket* peer);
};

#endif /* LOGINMANAGER_H_ */
