/*
 * MessengerEntity.h
 *
 * Provides basic I/O functions for client-server communication using control and
 * data channels.
 */

#ifndef MESSENGERENTITY_H_
#define MESSENGERENTITY_H_

#include <string>

#include "TCPSocket.h"

class MessengerEntity {
public:
	/**
	 * read command from peer
	 */
	static int readCommandFromPeer(TCPSocket* peer);

	/**
	 * read data from peer
	 */
	static std::string readDataFromPeer(TCPSocket* peer);

	/**
	 * send command to peer
	 */
	static void sendCommandToPeer(TCPSocket* peer, int command);

	/**
	 * send data to peer
	 */
	static void sendDataToPeer(TCPSocket* peer, std::string msg);
};

#endif /* MESSENGERENTITY_H_ */
