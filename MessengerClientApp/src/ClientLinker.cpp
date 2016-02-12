#include "ClientLinker.h"

ClientLinker::ClientLinker(int port) :
		_running(false), _clientSocket(port) {
	start();
}

ClientLinker::~ClientLinker() {
	_running = false;
	_clientSocket.cclose();
	waitForThread();
}

void ClientLinker::send(string msg, string ip, int port) {
	_clientSocket.sendTo(msg, ip, port);
}

void ClientLinker::run() {
	_running = true;

	while (_running) {
		char buffer[300] = {0};
		_clientSocket.recv(buffer, 300);
		cout << buffer << endl;
	}
}

