#include "MessengerServer.h"
#include "TCPMessengerProtocolExtentions.h"
#include "User.h"

User::User(string name, TCPSocket* sock) :
		_sock(sock) {
	string tempPort;
	istringstream liness(_sock->destIpAndPort());
	getline(liness, _ip, ':');
	getline(liness, tempPort, ':');
	_port = atoi(tempPort.c_str());

	_inChatRoom = false;
	_inSession = false;
	_name = name;
	_chatRoom = NULL;
	_ChatPartner = NULL;
}

User::~User() {
	delete (_sock);
}

TCPSocket* User::getSocket() {
	return _sock;
}

bool User::inChat() {
	return (inChatRoom() || inSession());
}

bool User::inChatRoom() {
	return _inChatRoom;
}

bool User::inSession() {
	return _inSession;
}

void User::loginUsertoSession(User* partner) {
	_inSession = true;
	_ChatPartner = partner;

}

void User::disconnectFromChatRom(bool fromchatroom) {
	if (inChatRoom()) {
		if (!fromchatroom)
			_chatRoom->logOffUser(this);
		writeCommand(CHAT_ROOM_USER_LEFT);
		_chatRoom = NULL;
		_inChatRoom = false;
	}

}

void User::loginUserToChatRoom(ChatRoom* logToRoom) {
	_inChatRoom = true;
	_chatRoom = logToRoom;
}

bool User::closeSession(bool isinitiating) {
	if (!inSession())
		return true;
	if (isinitiating)
		_ChatPartner->closeSession(false);

	writeCommand(SESSION_CLOSED);
	_inSession = false;
	_ChatPartner = NULL;
	return true;
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

string User::getDestandport() {
	return _sock->destIpAndPort();
}

string User::getusername() {
	return _name;
}

string User::getIP() {
	return _ip;
}

int User::getport() {
	return _port;
}

ChatRoom* User::getChatRoom() {
	return _chatRoom;
}
