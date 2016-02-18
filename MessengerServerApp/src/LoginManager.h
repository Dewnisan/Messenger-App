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
	std::map<std::string, TCPSocket*> _pendingPeers; // peers which havn't logged in yet
	MessengerServer* _messengerServer;

	/**
	 * Searches in users file for the given username and password and adds users
	 * to logged-in pool in the server.
	 */
	bool login(string name, string password, TCPSocket *sock);

	// Writes to users file the new user with his details
	bool signUp(string name, string password);

public:

	LoginManager(MessengerServer* messengerServer);
	virtual ~LoginManager();

	void run();

	/**
	 * Adds a client who connected to the server to the pool of clients who
	 * havn't logged in yet
	 */
	void addPendingPeer(TCPSocket* peer);
};

#endif /* LOGINMANAGER_H_ */
