/*
 * Listener.h
 *
 *  Created on: Feb 6, 2016
 *      Author: user
 */

#ifndef LISTENER_H_
#define LISTENER_H_

#include "LoginManager.h"
#include "TCPSocket.h"

class Listener: public MThread {
	LoginManager* _loginManager;
	TCPSocket* _clientSocket;
	bool _running;

public:

	Listener(LoginManager* registration);
	void run();
	virtual ~Listener();
};

#endif /* LISTENER_H_ */
