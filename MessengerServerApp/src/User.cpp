#include "MessengerServer.h"
#include "TCPMessengerProtocolExtentions.h"

#include "User.h"

using namespace std;

User::User(string name, TCPSocket* sock) :
		_name(name), _inSession(false), _inChatRoom(false), _sock(sock), _chatPartner(NULL), _chatRoom(NULL) {
	istringstream liness(_sock->destIpAndPort());
	getline(liness, _ip, ':');

	string tempPort;
	getline(liness, tempPort, ':');

	_port = atoi(tempPort.c_str());
}

User::~User() {
	delete (_sock);
}

bool User::inChatRoom() {
	return _inChatRoom;
}

bool User::inSession() {
	return _inSession;
}

bool User::isConversing() {
	return (inChatRoom() || inSession());
}

string User::getName() {
	return _name;
}

string User::getIp() {
	return _ip;
}

int User::getPort() {
	return _port;
}

ChatRoom* User::getChatRoom() {
	return _chatRoom;
}

void User::pairToSession(User* partner) {
	_inSession = true;
	_chatPartner = partner;
}

bool User::closeSession(bool isInitiating) {
	if (!inSession()) {
		return true;
	}

	if (isInitiating) {
		_chatPartner->closeSession(false);
	}

	writeCommand(SESSION_CLOSED);
	_inSession = false;
	_chatPartner = NULL;

	return true;
}

void User::enterToChatRoom(ChatRoom* name) {
	_inChatRoom = true;
	_chatRoom = name;
}

void User::exitChatRoom(bool fromChatRoom) {
	if (inChatRoom()) {
		if (!fromChatRoom) {
			_chatRoom->removeUser(this);
		}

		writeCommand(CHAT_ROOM_USER_LEFT);
		_chatRoom = NULL;
		_inChatRoom = false;
	}
}

TCPSocket* User::getSocket() {
	return _sock;
}

string User::getDestAndPort() {
	return _sock->destIpAndPort();
}

int User::readCommand() {
	return MessengerServer::readCommandFromPeer(_sock);
}

string User::readMsg() {
	return MessengerServer::readDataFromPeer(_sock);
}

void User::writeMsg(string msg) {
	MessengerServer::sendDataToPeer(_sock, msg);
}

void User::writeCommand(int command) {
	MessengerServer::sendCommandToPeer(_sock, command);
}
