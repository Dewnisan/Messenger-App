#ifndef USER_H_
#define USER_H_

#include "TCPSocket.h"
#include "ChatRoom.h"
#include "TCPMessengerProtocol.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>

class ChatRoom;

class User{

	string _name;
	string _ip;
	int _port;
	bool _inChatRoom;
	bool _inSession;
	TCPSocket* _sock;
	User *_ChatPartner;
	ChatRoom *_chatRoom;

public:
	User(string name, TCPSocket* basesock);

	//User related functions
	bool inChat();
	bool inSession();
	bool inChatRoom();
	void loginUsertoSession(User* partner);
	void loginUserToChatRoom(ChatRoom* name);
	void disconnectFromChatRom(bool fromchatroom);
	bool closeSession(bool isinitiating);
	string getusername();
	string getIP();
	int getport();
	ChatRoom* getChatRoom();

	//SOCKET related functions
	TCPSocket* getSocket();
	string getDestandport();
	int readCommand();
	string readMsg();
	void writeMsg(string msg);
	void writeCommand(int command);

	virtual ~User();
};

#endif /* USER_H_ */
