#include <iostream>

#include "Listener.h"
#include "TCPMessengerProtocolExtentions.h"

using namespace std;

Listener::Listener(LoginManager* loginManager) :
		_loginManager(loginManager), _running(false), _clientSocket(new TCPSocket(MSNGR_PORT)) {
	start();
	cout << "Listener is up!" << endl;
}

Listener::~Listener() {
	_running = false;
	_clientSocket->cclose();
	waitForThread();
}

void Listener::run() {
	_running = true;

	while (_running) {
		TCPSocket* temp = _clientSocket->listenAndAccept();
		if (temp == NULL) {
			break;
		}

		_loginManager->addPeer(temp);
		cout << "New peer connected: " << temp->destIpAndPort() << endl;
	}

	cout << "Listener has stopped" << endl;
}

