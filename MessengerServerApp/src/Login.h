#ifndef LOGIN_H_
#define LOGIN_H_

#include "MThread.h"
#include <map>
#include <string>
#include "TCPSocket.h"
#include "MessengerServer.h"
#include "MultipleTCPSocketsListener.h"
#include "TCPMessengerProtocol.h"

using namespace std;

/*
 * This class handles the login requests and the registration requests
 */

class Login: public MThread
{
	const char* _loginFile = "connections.txt";
	map<string, TCPSocket*> peers; // peers who didn't logged in
	bool running;
	MultipleTCPSocketsListener *listener;
	MessengerServer* _serverManager;

	bool login(string userName,string password);
	bool signUp(string userName,string password);

public:

	Login(MessengerServer* _serverManager);
	void run();
	void addPeer(TCPSocket* peer);
	virtual ~Login();
};



class Connection: public MThread
{
	Login* registration;
	TCPSocket * clientSocket;
	bool running;

public:

	Connection(Login* registration);
	void run();
	virtual ~Connection();
};

#endif /* LOGIN_H_ */
