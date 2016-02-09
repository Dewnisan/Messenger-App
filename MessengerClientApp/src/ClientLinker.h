

#ifndef CLIENTLINKER_H_
#define CLIENTLINKER_H_

#include "MThread.h"
#include "UDPSocket.h"

class ClientLinker: public MThread {
	UDPSocket _clientSocket;
	bool _running;

public:
	void send(string msg,string IP, int port);
	ClientLinker(int port);
	void run();
	virtual ~ClientLinker();
};

#endif /* CLIENTLINKER_H_ */
