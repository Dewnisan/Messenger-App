#include <iostream>
#include <pthread.h>
#include <string>

#include "MultipleTCPSocketsListener.h"
#include "TCPMessengerProtocolExtentions.h"

#include "MessengerServer.h"

using namespace std;

void MessengerServer::printToScreen(string msgToScreen) {
	cout << msgToScreen << endl;
}

int MessengerServer::getNumOfUsersFromFile() {
	fstream loginFile;
	loginFile.open(_pathToUsersFile.c_str(), ios::in | ios::out);

	int counter = 0;
	while (!loginFile.eof()) {
		string line;
		getline(loginFile, line);

		if (line != "") {
			counter++;
		}
	}

	loginFile.close();

	return counter;
}

void MessengerServer::readFromFile(User *clientName) {
	fstream loginFile;
	loginFile.open(_pathToUsersFile.c_str(), ios::in | ios::out);

	if (loginFile.is_open()) {
		while (!loginFile.eof()) {
			string line;
			getline(loginFile, line);

			istringstream liness(line);
			string userFromFile;
			getline(liness, userFromFile, '-');

			if (clientName != NULL) {
				clientName->writeMsg(userFromFile); // Send to client who requested
			} else {
				printToScreen(userFromFile); // Print on server's screen
			}

		}

		loginFile.close();
	} else {
		printToScreen("Error - could not open the file");
	}
}

void MessengerServer::readFromChatRoom(User *clientName) {
	for (map<string, ChatRoom*>::iterator iter = _chatRooms.begin(); iter != _chatRooms.end(); iter++) {
		string name = iter->first;

		if (clientName != NULL) {
			clientName->writeMsg(name); // send to client who requested
		} else {
			printToScreen(name); // Print on server's screen
		}
	}
}

void MessengerServer::createSession(User* fromUser, User* toUser) {
	// login the two users
	toUser->pairToSession(fromUser);
	fromUser->pairToSession(toUser);

	// send communication details
	toUser->writeCommand(SESSION_ESTABLISHED);
	toUser->writeMsg(fromUser->getName());
	toUser->writeMsg(fromUser->getIp());
	toUser->writeCommand(fromUser->getPort());
	toUser->writeCommand(toUser->getPort());

	fromUser->writeCommand(SESSION_ESTABLISHED);
	fromUser->writeMsg(toUser->getName());
	fromUser->writeMsg(toUser->getIp());
	fromUser->writeCommand(toUser->getPort());
	fromUser->writeCommand(fromUser->getPort());

	cout << "Session was created between " << fromUser->getName() << " and " << toUser->getName() << endl;
}

void MessengerServer::run() {
	_running = true;

	while (_running) {
		MultipleTCPSocketsListener multipleSocketsListener;

		// Convert User to socket before adding to pool
		vector<TCPSocket*> sockets;
		for (map<string, User*>::iterator iter = _users.begin(); iter != _users.end(); iter++) {
			sockets.push_back(iter->second->getSocket());
		}

		multipleSocketsListener.addSockets(sockets);

		// Listen to sockets
		TCPSocket* readySock = multipleSocketsListener.listenToSocket(2);
		if (readySock == NULL) {
			continue;
		}

		// Find matching user
		User* currUser = NULL;
		for (map<string, User*>::iterator iter = _users.begin(); iter != _users.end(); iter++) {
			if (readySock == iter->second->getSocket()) {
				currUser = iter->second;
				break;
			}
		}

		string msg;
		// Read command from the user
		int command = currUser->readCommand();
		switch (command) {
		case SESSION_CREATE:
			msg = currUser->readMsg(); // the partner name
			if (_users.find(msg) == _users.end()) {
				currUser->writeCommand(SESSION_CREATE_REFUSED);
				currUser->writeMsg("the requested user is disconnected");
				break;
			} else if (_users[msg]->isConversing()) {
				currUser->writeCommand(SESSION_CREATE_REFUSED);
				currUser->writeMsg("the requested user is in chat");
				break;
			}

			createSession(currUser, _users[msg]);
			break;
		case EXIT:
			exitServer(currUser);
			break;
		case SESSION_CLOSE:
			currUser->closeSession(true);
			break;
		case CHAT_ROOM_CREATE:
			createChatRoom(currUser);
			break;
		case CHAT_ROOM_CLOSE:
			deleteChatRoom(currUser);
			break;
		case CHAT_ROOM_ENTER:
			enterChatRoom(currUser);
			break;
		case CHAT_ROOM_EXIT:
			currUser->exitChatRoom(false);
			break;
		case LIST_CONNECTED_USERS:
			sendListConnectedUsers(currUser);
			break;
		case LIST_CHAT_ROOMS:
			sendListChatRooms(currUser);
			break;
		case LIST_CONNECTED_USERS_IN_CHAT_ROOM:
			sendListChatRoomUsers(currUser);
			break;
		case LIST_USERS:
			sendListUsers(currUser);
			break;
		}
	}
}

MessengerServer::MessengerServer(const std::string& pathToUsersFile) :
		_running(false), _pathToUsersFile(pathToUsersFile) {
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);

	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&_lock, &attr);

	pthread_mutexattr_destroy(&attr);

	start();
	cout << "Server is up" << endl;
}

MessengerServer::~MessengerServer() {
	_running = false;
	waitForThread();
	pthread_mutex_destroy(&_lock);
}

void MessengerServer::listUsers() {
	readFromFile(NULL); // To server
}

void MessengerServer::listConnectedUsers() {
	if (_users.empty()) {
		cout << "There are no users connected" << endl;
		return;
	}

	cout << "Connected users:" << endl;
	for (map<string, User*>::iterator iter = _users.begin(); iter != _users.end(); iter++) {
		string name = iter->first;
		printToScreen(name);
	}
}

void MessengerServer::listSessions() {
	bool found = false;

	for (map<string, User*>::iterator iter = _users.begin(); iter != _users.end(); iter++) {
		if (iter->second->inSession()) {
			found = true;
			string name = iter->first;
			printToScreen(name);
		}
	}

	if (!found) {
		cout << "There are no open sessions" << endl;
	}
}

void MessengerServer::listChatRooms() {
	if (_chatRooms.empty()) {
		cout << "There are no chat rooms" << endl;
		return;
	}

	readFromChatRoom(NULL);
}

void MessengerServer::listChatRoomUsers(string chatRoomName) {
	if (_chatRooms.find(chatRoomName) == _chatRooms.end()) {
		cout << "Chat room does not exist" << endl;
		return;
	}

	int numOfUsers = 0;
	for (map<string, ChatRoom*>::iterator iter = _chatRooms.begin(); iter != _chatRooms.end(); iter++) {
		if (chatRoomName == iter->first) {
			numOfUsers = iter->second->printUsers();
		}
	}

	if (numOfUsers == 0) {
		cout << "There are no users in this chat room" << endl;
	}
}

bool MessengerServer::addUser(string userName, TCPSocket* userSocket) {
	User* userToAdd = new User(userName, userSocket);

	pair<map<string, User*>::iterator, bool> ret = _users.insert(pair<string, User*>(userName, userToAdd));

	if (ret.second) {
		cout << userName << " has logged in" << endl;
	} else {
		cout << "Could not add the user " + userName + " to users pool" << endl;
	}

	return ret.second;
}

void MessengerServer::sendListConnectedUsers(User *client) {
	client->writeCommand(LIST_CONNECTED_USERS);
	client->writeCommand(_users.size());

	for (map<string, User*>::iterator iter = _users.begin(); iter != _users.end(); iter++) {
		string name = iter->first;
		client->writeMsg(name);
	}
}

bool MessengerServer::isLoggedIn(string userName) {
	if (_users.find(userName) == _users.end()) { // not found
		return false;
	}

	return true;
}

void MessengerServer::sendListChatRooms(User *client) {
	client->writeCommand(LIST_CHAT_ROOMS);
	client->writeCommand(_chatRooms.size());

	readFromChatRoom(client);
}

void MessengerServer::sendListChatRoomUsers(User *client) {
	string chatRoomName = client->readMsg();
	map<string, ChatRoom*>::iterator iter = _chatRooms.find(chatRoomName);

	if (iter != _chatRooms.end()) {
		client->writeCommand(LIST_CONNECTED_USERS_IN_CHAT_ROOM);
		iter->second->sendUserList(client);
	} else {
		client->writeCommand(CHAT_ROOM_NOT_EXIST);
	}
}

void MessengerServer::sendListUsers(User *client) {
	client->writeCommand(LIST_USERS);

	int numOfUsers = getNumOfUsersFromFile();
	client->writeCommand(numOfUsers);
	readFromFile(client);
}

void MessengerServer::exitServer(User* client) {
	cout << "The user " << client->getName() << " was disconnected" << endl;
	_users.erase(client->getName());
}

void MessengerServer::createChatRoom(User* creator) {
	string msg;
	bool exist = false;
	msg = creator->readMsg();

	// Checking that the name does not exist
	for (map<string, ChatRoom*>::iterator iter = _chatRooms.begin(); iter != _chatRooms.end(); iter++) {
		if (iter->first == msg) {
			exist = true;
		}
	}

	if (exist) {
		creator->writeCommand(CHAT_ROOM_CREATE_DENIED);
		creator->writeMsg(string("Chat Room name already exists"));
		return;
	}

	//Add new chat room
	_chatRooms[msg] = new ChatRoom(creator, msg);
	creator->writeCommand(CHAT_ROOM_CREATED);
	cout << "Room " << msg << " was created successfully" << endl;
}

void MessengerServer::deleteChatRoom(User* creator) {
	string msg;
	msg = creator->readMsg();

	bool exist = false;

	for (map<string, ChatRoom*>::iterator iter = _chatRooms.begin(); iter != _chatRooms.end(); iter++) {
		if (iter->first == msg) {
			exist = true;
		}
	}

	if (!exist) {
		creator->writeCommand(CHAT_ROOM_UNCLOSED);
	}

	if (_chatRooms[msg]->getOwner()->getName() == creator->getName()) {
		delete (_chatRooms[msg]);
		creator->writeCommand(CHAT_ROOM_CLOSED);
		_chatRooms.erase(msg);
	} else {
		creator->writeCommand(CHAT_ROOM_UNCLOSED);
	}

}

void MessengerServer::enterChatRoom(User* user) {
	string roomName = user->readMsg();

	if (_chatRooms.find(roomName) == _chatRooms.end()) {
		user->writeCommand(CHAT_ROOM_ENTERING_DENIED);
		user->writeMsg(string("Room does not exist"));
		return;
	}

	user->enterToChatRoom(_chatRooms[roomName]);
	if (_chatRooms[roomName]->addUser(user)) {
		user->writeCommand(CHAT_ROOM_USER_ENTERED);
		user->writeCommand(user->getPort());
	} else {
		user->writeCommand(CHAT_ROOM_ENTERING_DENIED);
		user->writeMsg(string("You are already in the chat room"));
	}
}

bool MessengerServer::addUserToFile(string name, string password) {
	bool retval = false;
	fstream loginFile;

	pthread_mutex_lock(&_lock);

	if (!isUserExistsInFile(name)) {
		ofstream usersFile;
		usersFile.open(_pathToUsersFile.c_str(), ios::app);

		if (usersFile.is_open()) {
			usersFile << name + "-" + password << endl;
			usersFile.close();
			retval = true;
		} else {
			cout << "Error - could not open file!" << endl;
		}
	} else {
		cout << "User already exists in file" << endl;
	}

	pthread_mutex_unlock(&_lock);

	return retval;
}

bool MessengerServer::isUserExistsInFile(string name) {
	bool retval = false;
	fstream usersFile;

	pthread_mutex_lock(&_lock);

	usersFile.open(_pathToUsersFile.c_str(), ios::in | ios::out);

	if (usersFile.is_open()) {
		while (!usersFile.eof()) {
			string line;
			getline(usersFile, line);

			string delimiter("-");
			string::size_type delimiterIndex = line.find(delimiter);
			string nameFromFile = line.substr(0, delimiterIndex);
			if (nameFromFile == name) {
				retval = true;
			}
		}

		usersFile.close();
	} else {
		cout << "Error - could not open file!" << endl;
	}

	pthread_mutex_unlock(&_lock);

	return retval;
}

bool MessengerServer::isLoginInfoExistsInFile(string name, string password) {
	bool retval = false;
	fstream usersFile;

	pthread_mutex_lock(&_lock);

	usersFile.open(_pathToUsersFile.c_str(), ios::in | ios::out);

	if (usersFile.is_open()) {
		while (!usersFile.eof()) {
			string line;
			getline(usersFile, line);

			if (line == name + "-" + password) {
				return true;
			}
		}

		usersFile.close();
	} else {
		cout << "Error - could not open file!" << endl;
	}

	pthread_mutex_unlock(&_lock);

	return retval;
}
