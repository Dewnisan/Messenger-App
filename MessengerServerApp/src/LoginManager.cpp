#include <string>

#include "TCPSocket.h"
#include "MessengerServer.h"
#include "MultipleTCPSocketsListener.h"
#include "TCPMessengerProtocolExtentions.h"

#include "LoginManager.h"

using namespace std;

bool LoginManager::login(string name, string password) {
	return _messengerServer->isUserExistsInFile(name, password);
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

	for (map<string, TCPSocket*>::iterator iter = _peers.begin(); iter != _peers.end(); iter++) {
		iter->second->cclose();
	}

	waitForThread();
}

void LoginManager::run() {
	while (_running) {
		MultipleTCPSocketsListener multipleSocketsListener;

		vector<TCPSocket*> sockets;
		for (map<std::string, TCPSocket*>::iterator iter = _peers.begin(); iter != _peers.end(); iter++) {
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
			if (login(username, password)) { // Username and password already exist
				if (_messengerServer->isConnected(username)) { // Already logged in
					MessengerServer::sendCommandToPeer(readySock, LOGIN_REQUEST_ALREADY_LOGGED);
				} else {
					_messengerServer->addUser(readySock, username);
					_peers.erase(readySock->destIpAndPort());
					MessengerServer::sendCommandToPeer(readySock, LOGIN_REQUEST_APPROVED);
				}
			} else { // Wrong details
				MessengerServer::sendCommandToPeer(readySock, LOGIN_REQUEST_WRONG_DETAILS);
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
			_peers.erase(readySock->destIpAndPort());
			cout << "Peer disconnected: " + readySock->destIpAndPort() << endl;
			break;
		}
	}
}

void LoginManager::addPeer(TCPSocket* peer) {
	_peers[peer->destIpAndPort()] = peer;
}

