#include <algorithm>

#include "ChatRoom.h"
#include "TCPMessengerProtocolExtentions.h"

using namespace std;

ChatRoom::ChatRoom(User* owner, string name) :
		_name(name), _owner(owner) {
}

ChatRoom::~ChatRoom() {
	for (vector<User*>::iterator iter = _users.begin(); iter != _users.end(); iter++) {
		(*iter)->exitChatRoom(true);
	}
}

User* ChatRoom::getOwner() {
	return _owner;
}

void ChatRoom::updateUsers() {
	for (vector<User*>::iterator iter = _users.begin(); iter != _users.end(); iter++) {
		(*iter)->writeCommand(CHAT_ROOM_UPDATED);
		(*iter)->writeMsg(_name);
		(*iter)->writeCommand(_users.size());

		for (vector<User*>::iterator iter2 = _users.begin(); iter2 != _users.end(); iter2++) {
			(*iter)->writeMsg((*iter2)->getName());
			(*iter)->writeMsg((*iter2)->getIp());
			(*iter)->writeCommand((*iter2)->getPort());
		}
	}
}

bool ChatRoom::addUser(User* user) {
	if (find(_users.begin(), _users.end(), user) != _users.end()) {
		cout << "User already exists" << endl;
		return false;
	}

	_users.push_back(user);
	updateUsers();

	return true;
}

bool ChatRoom::removeUser(User *user) {
	vector<User*>::iterator iter = find(_users.begin(), _users.end(), user);
	if (iter == _users.end()) {
		cout << "User does not exists" << endl;
		return false;
	}

	_users.erase(iter);
	updateUsers();

	return true;
}

int ChatRoom::printUsers() {
	int count = 0;
	for (vector<User*>::iterator iter = _users.begin(); iter != _users.end(); iter++) {
		cout << (*iter)->getName() << endl;
		count++;
	}

	return count;
}

void ChatRoom::sendUserList(User *client) {
	client->writeCommand(_users.size());
	for (vector<User*>::iterator iter = _users.begin(); iter != _users.end(); iter++) {
		client->writeMsg((*iter)->getName());
	}
}

