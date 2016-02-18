/*
 * Listener.h
 *
 * Listens and accepts incoming connections to the server.
 */

#ifndef LISTENER_H_
#define LISTENER_H_

#include "LoginManager.h"
#include "TCPSocket.h"

class Listener: public MThread {
	bool _running;
	LoginManager* _loginManager;
	TCPSocket* _listenSock;

public:

	Listener(LoginManager* registration);
	virtual ~Listener();

	// Waits for connection from new peers and adds them to the peer pool
	void run();

};

#endif /* LISTENER_H_ */
