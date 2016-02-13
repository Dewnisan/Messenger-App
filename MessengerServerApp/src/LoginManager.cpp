#include <string>

#include "LoginManager.h"
#include "MultipleTCPSocketsListener.h"
#include "TCPMessengerProtocolExtentions.h"

using namespace std;

bool LoginManager::login(string userName, string password) {
	fstream loginFile;
	loginFile.open(_loginFile, ios::in | ios::out | ios::binary);

	if (loginFile.is_open()) {
		while (!loginFile.eof()) {
			string line;
			getline(loginFile, line);

			if (line == userName + string("-") + password) {
				return true;
			}
		}

		loginFile.close();
	} else {
		cout << "Error - could not open file!" << endl;
	}

	return false;
}

bool LoginManager::signUp(string userName, string password) {
	fstream loginFile;
	loginFile.open(_loginFile, ios::in | ios::out | ios::binary);

	if (!loginFile.is_open()) {
		cout << "Error - could not open file!" << endl;
		return false;
	}

	while (!loginFile.eof()) {
		string line;
		getline(loginFile, line);

		string userFromFile;
		istringstream liness(line);
		getline(liness, userFromFile, ':');

		if (userFromFile == userName) {
			loginFile.close();
			return false;
		}
	}

	loginFile.close();

	ofstream loginFile1;
	loginFile1.open(_loginFile, ios::app);
	loginFile1 << userName + string("-") + password << endl;
	loginFile1.close();

	return true;
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

		int command = readySock->readCommand();
		switch (command) {
		case LOGIN_REQUEST:
			username = readySock->readMsg();
			password = readySock->readMsg();
			if (login(username, password)) { // Username and password already exist
				if (_messengerServer->isConnected(username)) { // Already logged in
					readySock->writeCommand(LOGIN_REQUEST_ALREADY_LOGGED);
				} else {
					_messengerServer->addUser(readySock, username);
					_peers.erase(readySock->destIpAndPort());
					readySock->writeCommand(LOGIN_REQUEST_APPROVED);
				}
			} else { // Wrong details

				readySock->writeCommand(LOGIN_REQUEST_WRONG_DETAILS);
			}
			break;

		case REGISTRATION_REQUEST:
			username = readySock->readMsg();
			password = readySock->readMsg();
			if (signUp(username, password)) {
				readySock->writeCommand(REGISTRATION_REQUEST_APPROVED);
			} else {
				readySock->writeCommand(REGISTRATION_REQUEST_DENIED);
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

