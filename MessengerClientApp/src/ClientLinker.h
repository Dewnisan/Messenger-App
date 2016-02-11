
#ifndef CLIENTLINKER_H_
#define CLIENTLINKER_H_

#include <string>

#include "MThread.h"
#include "UDPSocket.h"

class ClientLinker: public MThread {
	UDPSocket _clientSocket;
	bool _running;

public:
	ClientLinker(int port);
	virtual ~ClientLinker();

	void send(std::string msg, std::string IP, int port);

	/**
	 * This method runs in a separate thread, it reads the incoming messages
	 * from the socket.
	 * The thread should exist when the socket is closed
	 */
	void run();
};

#endif /* CLIENTLINKER_H_ */
