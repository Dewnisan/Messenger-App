#ifndef USER_H_
#define USER_H_

/**
 * Represents a user who is logged into the server.
 */

#include <fstream>
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <sstream>
#include <string>

#include "ChatRoom.h"
#include "TCPSocket.h"

class ChatRoom;

class User {
	string _name;
	string _ip;
	int _port;

	bool _inSession;
	bool _inChatRoom;

	TCPSocket* _sock;

	User *_chatPartner;
	ChatRoom *_chatRoom;

public:
	User(string name, TCPSocket* sock);
	virtual ~User();

	// User related functions
	bool inSession();
	bool inChatRoom();
	bool isConversing();

	string getName();
	string getIp();
	int getPort();
	ChatRoom* getChatRoom();

	void pairToSession(User* partner);
	bool closeSession(bool isInitiating);

	void enterToChatRoom(ChatRoom* name);
	void exitChatRoom(bool fromChatRoom);

	// Socket related functions
	TCPSocket* getSocket();
	string getDestAndPort();

	int readCommand();
	string readMsg();
	void writeMsg(string msg);
	void writeCommand(int command);
};

#endif /* USER_H_ */
