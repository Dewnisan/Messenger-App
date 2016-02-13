/*
 * MessengerEntity.cpp
 *
 *  Created on: Feb 9, 2016
 *      Author: user
 */

#include <string>

#include "MessengerEntity.h"
#include "TCPSocket.h"

int MessengerEntity::readCommandFromPeer(TCPSocket* peer) {
	// read a command from socket
	int command = 0;
	int byteCount = peer->recv((char*)&command, 4);
	if (byteCount < 4) {
		cout << "Error while reading command from peer" << endl;
		return byteCount;
	}

	command = ntohl(command);
	return command;
}

string MessengerEntity::readDataFromPeer(TCPSocket* peer) {
	// read a string from socket
	int messageLength = 0;
	peer->recv((char*)&messageLength, 4);
	messageLength = ntohl(messageLength);

	char buffer[1500];
	int totalByteCount = 0;
	int byteCount = 0;
	while (totalByteCount < messageLength) {
		byteCount = peer->recv((char*)&buffer[totalByteCount], messageLength - totalByteCount);
		if (byteCount > 0) {
			totalByteCount += byteCount;
		} else {
			break;
		}
	}

	string msg;
	if (byteCount > 0 && totalByteCount == messageLength) {
		buffer[messageLength] = 0;
		msg = buffer;
	} else {
		peer->cclose();
	}

	return msg;
}

void MessengerEntity::sendCommandToPeer(TCPSocket* peer, int command) {
	// send command to socket
	command = htonl(command);
	peer->send((char*)&command, 4);
}

void MessengerEntity::sendDataToPeer(TCPSocket* peer, string msg) {
	// send string to socket
	int messageLength = htonl(msg.length());
	peer->send((char*)&messageLength, 4);
	peer->send(msg.data(), msg.length());
}
