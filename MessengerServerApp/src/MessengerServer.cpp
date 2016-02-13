#include <iostream>
#include <string>

#include "MessengerServer.h"
#include "MultipleTCPSocketsListener.h"
#include "TCPMessengerProtocolExtentions.h"

using namespace std;

void MessengerServer::printToScreen(string msgToScreen) {
	cout << msgToScreen << endl;
}

int MessengerServer::numOfUsersFromFile() {
	fstream loginFile;
	loginFile.open(_pathToUsersFile.c_str(), ios::in | ios::out | ios::binary);

	int counter = 0;
	while (!loginFile.eof()) {
		string line;
		getline(loginFile, line);
		counter++;
	}

	loginFile.close();

	return counter;
}

void MessengerServer::readFromFile(User *clientName) {
	fstream loginFile;
	loginFile.open(_pathToUsersFile.c_str(), ios::in | ios::out | ios::binary);

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
}

void MessengerServer::run() {
	_running = true;

	while (_running) {
		MultipleTCPSocketsListener multipleSocketsListener;

		// Convert User to socket before adding to list
		vector<TCPSocket*> sockets;
		for (map<string, User*>::iterator iter = _users.begin(); iter != _users.end(); iter++) {
			sockets.push_back(iter->second->getSocket());
		}

		// Add sockets
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
			if (!_users[msg]) {
				currUser->writeCommand(SESSION_CREATE_REFUSED);
				currUser->writeMsg(string("there is no such user"));
				break;
			} else if (_users[msg]->isConversing()) {
				currUser->writeCommand(SESSION_CREATE_REFUSED);
				currUser->writeMsg(string("the wanted user is in chat"));
				break;
			}
			createSession(currUser, _users[msg]);
			cout << "Session was created between: " << currUser->getName() << " AND " << msg << endl;
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
		case CHAT_ROOM_LOGIN:
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
	start();
	cout << "Server is up" << endl;
}

MessengerServer::~MessengerServer() {
	_running = false;
	waitForThread();
}

void MessengerServer::listUsers() {
	cout << "Users:" << endl;
	readFromFile(NULL); // To server
}

void MessengerServer::listConnectedUsers() {
	if (_users.empty()) {
		cout << "There aren't any users connected" << endl;
		return;
	}

	cout << "Connected users:" << endl;
	for (map<string, User*>::iterator iter = _users.begin(); iter != _users.end(); iter++) {
		string name = iter->first;
		printToScreen(name);
	}
}

void MessengerServer::listSessions() {
	cout << "Connected users that in session:" << endl;

	for (map<string, User*>::iterator iter = _users.begin(); iter != _users.end(); iter++) {
		if (iter->second->inSession()) {
			string name = iter->first;
			printToScreen(name);
		}
	}
}

void MessengerServer::listChatRooms() {
	if (_chatRooms.empty()) {
		cout << "There are no chat rooms yet" << endl;
		return;
	}

	cout << "Chat rooms:" << endl;
	readFromChatRoom(NULL);
}

void MessengerServer::listChatRoomUsers(string chatRoomName) {
	if (_chatRooms.find(chatRoomName) == _chatRooms.end()) {
		cout << "No such chat room: " << chatRoomName << endl;
		return;
	}

	cout << "Users in chat room:" << endl;
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

bool MessengerServer::addUser(TCPSocket* userSocket, string userName) {
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

bool MessengerServer::isConnected(string userName) {
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
	for (map<string, ChatRoom*>::iterator iter = _chatRooms.begin(); iter != _chatRooms.end(); iter++) {
		if (chatRoomName == iter->first) {
			client->writeCommand(LIST_CONNECTED_USERS_IN_CHAT_ROOM);
			iter->second->sendUserList(client);
			break;
		}
	}
}

void MessengerServer::sendListUsers(User *client) {
	client->writeCommand(LIST_USERS);

	int numOfUsers = numOfUsersFromFile();
	client->writeCommand(numOfUsers - 1); // TODO: why?

	if (client != NULL) {
		readFromFile(client);
	} else {
		readFromFile(NULL);
	}
}

void MessengerServer::exitServer(User* client) {
	// TODO: what about these?
//	client->closeSession(true);
//	client->disconnectFromChatRom(false);

	cout << "The user: " << client->getName() << " was disconnected" << endl;
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
	cout << "Room : " << msg << " was created" << endl;
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

void MessengerServer::enterChatRoom(User* loginUser) {
	string roomName = loginUser->readMsg();
	bool exist = false;
	for (map<string, ChatRoom*>::iterator iter = _chatRooms.begin(); iter != _chatRooms.end(); iter++) {
		if (iter->first == roomName) {
			exist = true;
		}
	}

	if (!exist) {
		loginUser->writeCommand(CHAT_ROOM_ENTERING_DENIED);
		loginUser->writeMsg(string("Room does not exist"));
		return;
	}

	loginUser->enterToChatRoom(_chatRooms[roomName]);
	if (_chatRooms[roomName]->addUser(loginUser)) // addUser of ChatRoom
			{
		loginUser->writeCommand(CHAT_ROOM_USER_ENTERED);
		loginUser->writeCommand(loginUser->getPort());
	} else {
		loginUser->writeCommand(CHAT_ROOM_ENTERING_DENIED);
		loginUser->writeMsg(string("you are already logged in"));
	}
}
