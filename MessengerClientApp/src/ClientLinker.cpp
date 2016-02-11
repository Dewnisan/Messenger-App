#include "ClientLinker.h"

ClientLinker::ClientLinker(int port):_clientSocket(port)
{
	start();
}

ClientLinker::~ClientLinker() {
	_running = false;
	_clientSocket.cclose();
	waitForThread();
}

void ClientLinker::send(string msg,string IP, int port)
{
	_clientSocket.sendTo(msg,IP,port);
}

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


