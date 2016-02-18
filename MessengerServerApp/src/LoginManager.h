/**
 * Handles the login and registration requests
 */

#ifndef LOGINMANAGER_H_
#define LOGINMANAGER_H_

#include <map>
#include <string>

#include "MessengerServer.h"
#include "MThread.h"
#include "TCPSocket.h"

class LoginManager: public MThread {
	bool _running;
	std::map<std::string, TCPSocket*> _pendingPeers; // peers which didn't log in
	MessengerServer* _messengerServer;

	// Search in file for the given username and password and adds users to logged-in pool.
	bool login(string name, string password, TCPSocket *sock);

	// Write to file the new user with his details
	bool signUp(string name, string password);

public:

	LoginManager(MessengerServer* messengerServer);
	virtual ~LoginManager();

	void run();
	void addPendingPeer(TCPSocket* peer);
};

#endif /* LOGINMANAGER_H_ */
