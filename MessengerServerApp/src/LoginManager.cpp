#include <string>

#include "TCPSocket.h"
#include "MessengerServer.h"
#include "MultipleTCPSocketsListener.h"
#include "TCPMessengerProtocolExtentions.h"

#include "LoginManager.h"

using namespace std;

bool LoginManager::login(string name, string password, TCPSocket *sock) {
	if (_messengerServer->isLoggedIn(name) || !_messengerServer->isUserExistsInFile(name, password)) {
		return false;
	}

	_messengerServer->addUser(name, sock);
	_pendingPeers.erase(sock->destIpAndPort());

	return true;
}

bool LoginManager::signUp(string name, string password) {
	return _messengerServer->addUserToFile(name, password);
}

LoginManager::LoginManager(MessengerServer* messengerServer) :
		_running(false), _messengerServer(messengerServer) {
	start();
}

LoginManager::~LoginManager() {
	_running = false;

	for (map<string, TCPSocket*>::iterator iter = _pendingPeers.begin(); iter != _pendingPeers.end(); iter++) {
		iter->second->cclose();
	}

	waitForThread();
}

void LoginManager::run() {
	while (_running) {
		MultipleTCPSocketsListener multipleSocketsListener;

		vector<TCPSocket*> sockets;
		for (map<std::string, TCPSocket*>::iterator iter = _pendingPeers.begin(); iter != _pendingPeers.end(); iter++) {
			sockets.push_back(iter->second);
		}

		multipleSocketsListener.addSockets(sockets);

		TCPSocket* readySock = multipleSocketsListener.listenToSocket(2);
		if (readySock == NULL) {
			continue;
		}

		string username;
		string password;

		int command = MessengerServer::readCommandFromPeer(readySock);
		switch (command) {
		case LOGIN_REQUEST:
			username = MessengerServer::readDataFromPeer(readySock);
			password = MessengerServer::readDataFromPeer(readySock);
			if (login(username, password, readySock)) {
				MessengerServer::sendCommandToPeer(readySock, LOGIN_REQUEST_APPROVED);
			} else {
				if (_messengerServer->isLoggedIn(username)) { // Already logged in
					MessengerServer::sendCommandToPeer(readySock, LOGIN_REQUEST_ALREADY_LOGGED);
				} else {
					MessengerServer::sendCommandToPeer(readySock, LOGIN_REQUEST_WRONG_DETAILS);
				}
			}
			break;

		case REGISTRATION_REQUEST:
			username = MessengerServer::readDataFromPeer(readySock);
			password = MessengerServer::readDataFromPeer(readySock);
			if (signUp(username, password)) {
				MessengerServer::sendCommandToPeer(readySock, REGISTRATION_REQUEST_APPROVED);
			} else {
				MessengerServer::sendCommandToPeer(readySock, REGISTRATION_REQUEST_DENIED);
			}
			break;

		case EXIT:
			_pendingPeers.erase(readySock->destIpAndPort());
			cout << "Peer disconnected: " + readySock->destIpAndPort() << endl;
			break;
		}
	}
}

void LoginManager::addPendingPeer(TCPSocket* peer) {
	_pendingPeers[peer->destIpAndPort()] = peer;
}

