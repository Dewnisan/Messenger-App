/*
 * Listener.cpp
 *
 *  Created on: Feb 6, 2016
 *      Author: user
 */

#include "Listener.h"

Listener::Listener(LoginManager* registration) {

	this->_loginManager = registration;
	_running = true;
	_clientSocket = new TCPSocket(MSNGR_PORT);
	start();
	cout << "server connection is up!" << endl;

}

// waiting for connection from new peers and adds them to the peer list
void Listener::run() {
	TCPSocket* temp;
	while (_running) {
		temp = _clientSocket->listenAndAccept();
		if (!temp)
			break;
		_loginManager->addPeer(temp);
		cout << "New peer connected: " << temp->destIpAndPort() << endl;
	}
	cout << "The connection with server has stopped" << endl;
}

Listener::~Listener() {
	_running = false;
	_clientSocket->cclose();
	waitForThread();
}

