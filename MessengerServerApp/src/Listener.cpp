#include <iostream>

#include "Listener.h"
#include "TCPMessengerProtocolExtentions.h"

using namespace std;

Listener::Listener(LoginManager* loginManager) :
		 _running(false), _loginManager(loginManager), _listenSock(new TCPSocket(MSNGR_PORT)) {
	start();
}

Listener::~Listener() {
	_running = false;
	_listenSock->cclose();
	waitForThread();
}

void Listener::run() {
	_running = true;

	while (_running) {
		TCPSocket* sock = _listenSock->listenAndAccept();
		if (sock == NULL) {
			break;
		}

		struct timeval timeout;
		timeout.tv_sec = 720;
		timeout.tv_usec = 0;

		setsockopt(sock->getSocketFid(), SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof timeout);

		_loginManager->addPendingPeer(sock);
		cout << "New peer connected: " << sock->destIpAndPort() << endl;
	}

	cout << "Listener has stopped" << endl;
}

