#include "ClientLinker.h"

ClientLinker::ClientLinker(int port):_clientSocket(port)
{
	start();
}

/**
 * This method runs in a separate thread, it reads the incoming messages
 * from the socket.
 * The thread should exist when the socket is closed
 */
void ClientLinker::run()
{
	_running = true;
	char buf[300];
	while(_running)
	{
		for(int i = 0;i<300;i++,buf[i] = 0);
		_clientSocket.recv(buf,300);
		cout<<buf<<endl;
	}
}

void ClientLinker::send(string msg,string IP, int port)
{
	_clientSocket.sendTo(msg,IP,port);
}

ClientLinker::~ClientLinker() {
	_running = false;
	_clientSocket.cclose();
	waitForThread();
}
