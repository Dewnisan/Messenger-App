#include "MessengerClient.h"

#include "TCPSocket.h"

MessengerClient::MessengerClient() {
	_loggedIn = false;
	_sessionOn = false;
	_roomOn = false;
	_connectedToServer = false;
	_serverSocket = NULL;
	_username.clear();
}

MessengerClient::~MessengerClient() {
	_running = false;
}

void MessengerClient::run() {
	string parameter1;
	int partnerPort;
	int command;
	_running = true;

	while (_running) {
		command = MessengerClient::readCommandFromPeer(_serverSocket);
		if (command == 0) {
			continue;
		}

		switch (command) {
		case SESSION_ESTABLISHED:
			_udpChatSideB._sideB_name = MessengerClient::readDataFromPeer(_serverSocket);
			_udpChatSideB._IP = MessengerClient::readDataFromPeer(_serverSocket);
			_udpChatSideB._port = MessengerClient::readCommandFromPeer(_serverSocket);

			partnerPort = MessengerClient::readCommandFromPeer(_serverSocket);
			_clientLinker = new ClientLinker(partnerPort);
			_sessionOn = true;

			cout << "You are in direct connection with  " << _udpChatSideB._sideB_name << endl;
			break;
		case SESSION_CREATE_REFUSED:
			parameter1 = MessengerClient::readDataFromPeer(_serverSocket);
			cout << "Session has been denied because " << parameter1 << endl;
			break;
		case SESSION_CLOSED:
			_udpChatSideB.clean();
			_sessionOn = false;
			delete (_clientLinker);
			cout << "the session is now terminated" << endl;
			break;
		case CHAT_ROOM_CREATE_DENIED:
			parameter1 = MessengerClient::readDataFromPeer(_serverSocket);
			cout << "room cannot be opened due to: " << parameter1 << endl;
			break;
		case CHAT_ROOM_CREATED:
			cout << "Chat room has been opened" << endl;
			break;
		case CHAT_ROOM_LEAVED:
			chatRoomLeaved();
			cout << "your user is out of the room" << endl;
			break;
		case CHAT_ROOM_LOGED_IN:
			partnerPort = MessengerClient::readCommandFromPeer(_serverSocket); // UDP listen port
			_clientLinker = new ClientLinker(partnerPort);
			_roomOn = true;
			cout << "You have joined to the room" << endl;
			break;
		case CHAT_ROOM_LOGED_IN_DENIED:
			cout << MessengerClient::readDataFromPeer(_serverSocket) << endl;
			break;
		case CHAT_ROOM_UPDATED:
			chatRoomUpdate();
			break;
		case CHAT_ROOM_CLOSED:
			cout << "The room has been closed." << endl;
			break;
		case CHAT_ROOM_UNCLOSED:
			cout << "You cannot delete the room." << endl;
			break;
		case LIST_CONNECTED_USERS:
			printConnectedUsers();
			break;
		case LIST_ROOMS:
			printRoomsList();
			break;
		case LIST_CONNECTED_USERS_IN_ROOM:
			printConnectedUsers();
			break;
		case LIST_USERS:
			printListUsers();
		}
	}
}

bool MessengerClient::connectToServer(string ip, int port) {
	if (_serverSocket != NULL) {
		return false;
	}

	_serverSocket = new TCPSocket(ip, port);
	_connectedToServer = true;

	return true;
}

void MessengerClient::signup(string username, string password, int cmd) {
	if (_loggedIn) {
		cout << "You cannot register while you logged in" << endl;
		return;
	}

	int response;
	if (_serverSocket) {
		MessengerClient::sendCommandToPeer(_serverSocket, cmd);
		MessengerClient::sendDataToPeer(_serverSocket, username);
		MessengerClient::sendDataToPeer(_serverSocket, password);

		response = MessengerClient::readCommandFromPeer(_serverSocket);
		if (response == REGISTRATION_REQUEST_APPROVED) {
			cout << "signed up was successful with the user: " << username << endl;
		} else if (cmd == REGISTRATION_REQUEST_DENIED) {
			cout << "you failed to sign up" << endl;
		}
	} else {
		cout << "the server is not connected" << endl;
	}
}

// login to server with a given username and password
void MessengerClient::login(string username, string password, int cmd) {
	if (_loggedIn) {
		cout << "login failed - you are already logged in" << endl;
		return;
	}

	int response;
	if (_serverSocket != NULL) {
		MessengerClient::sendCommandToPeer(_serverSocket, cmd);
		MessengerClient::sendDataToPeer(_serverSocket, username);
		MessengerClient::sendDataToPeer(_serverSocket, password);

		response = MessengerClient::readCommandFromPeer(_serverSocket);
		if (response == LOGIN_REQUEST_APPROVED) {
			_username = username;
			_loggedIn = true;
			start();
			cout << "you were logged in as " + username << endl;
		} else if (response == LOGIN_REQUEST_WRONG_DETAILS) {
			cout << "you enter wrong user or password" << endl;
		} else if (response == LOGIN_REQUEST_ALREADY_LOGGED) {
			cout << username + " already logged in" << endl;
		}
	} else {
		cout << "the server is not connected" << endl;
	}
}

bool MessengerClient::openSession(string partnerName) {
	if (_serverSocket == NULL || !_loggedIn) {
		cout << "You are not connected/logged in" << endl;
		return false;
	}

	if (isInChat()) {
		cout << "You are already in a session" << endl;
		return false;
	}

	if (partnerName == _username) {
		cout << "You can't open session with yourself" << endl;
		return false;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, SESSION_CREATE);
	MessengerClient::sendDataToPeer(_serverSocket, partnerName);

	return true;
}

void MessengerClient::printCurrentInfo() {
	if (_serverSocket)
		cout << "Connected to server " << endl;
	else
		cout << "NOT Connected to server " << endl;
	if (_loggedIn)
		cout << "Logged in as  " << _username << endl;
	else
		cout << "NOT logged in " << endl;
	if (_sessionOn)
		cout << "In session " << endl;
	else
		cout << "NOT in session " << endl;
	if (_roomOn)
		cout << "In chat room: " << _chatRoomName << endl;
	else
		cout << "NOT In chat room: " << endl;
}

// sending UDP message to specific user or to all the users in a room.
bool MessengerClient::sendMsg(string msg) {
	if (_sessionOn) {
		_clientLinker->send(string(">[") + _username + string("]") + msg, _udpChatSideB._IP, _udpChatSideB._port);
		return true;
	} else if (_roomOn) {
		std::vector<ChatSideB*>::iterator iter = _chatUsers.begin();
		std::vector<ChatSideB*>::iterator enditer = _chatUsers.end();

		while (iter != enditer) {
			_clientLinker->send(string(">[") + _username + string("] ") + msg, (*iter)->_IP, (*iter)->_port);
			iter++;
		}
		return true;
	}
	return false;
}

// open session in the given room name
bool MessengerClient::createChatRoom(string name) {
	if (!_serverSocket || !_loggedIn || isInChat()) {
		return false;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, CHAT_ROOM_CREATE);
	MessengerClient::sendDataToPeer(_serverSocket, name);

	return true;
}

bool MessengerClient::loginToChatRoom(string roomName) {
	if (isInChat() || !_serverSocket || !_loggedIn) {
		return false;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, CHAT_ROOM_LOGIN);
	MessengerClient::sendDataToPeer(_serverSocket, roomName);

	return true;
}

// print all of the connected users after received from the server
void MessengerClient::printConnectedUsers() {
	int i;
	int numOfUsers;
	numOfUsers = MessengerClient::readCommandFromPeer(_serverSocket);
	for (i = 0; i < numOfUsers; i++) {
		cout << " user: " << MessengerClient::readDataFromPeer(_serverSocket) << endl;
	}
	if (i == 0) {
		cout << "no one is connected" << endl;
	}
}

// print the list of the users from the file
void MessengerClient::printListUsers() {
	int i;
	int numOfUsers;
	numOfUsers = MessengerClient::readCommandFromPeer(_serverSocket);
	for (i = 0; i < numOfUsers; i++) {
		cout << " user: " << MessengerClient::readDataFromPeer(_serverSocket) << endl;
	}
	if (i == 0) {
		cout << "no one is connected" << endl;
	}
}

// send to the server request of connected users
void MessengerClient::printConnectedUsersRequest() {
	if (!_serverSocket || !_loggedIn) {
		cout << "You are not connected or not logged in" << endl;
		return;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, LIST_CONNECTED_USERS);
}

// send to the server request of list of the users from the file
void MessengerClient::listUsers() {
	if (!_serverSocket || !_loggedIn) {
		cout << "You are not connected or not logged in" << endl;
		return;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, LIST_USERS);
}

// send to the server request of room list
void MessengerClient::RoomsList() {
	if (!_serverSocket || !_loggedIn) {
		cout << "you are not connected or not logged in" << endl;
		return;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, LIST_ROOMS);
}

// send to the server request of all the users in a room (given by it's name)
void MessengerClient::listConnectedUsersInRoom(string roomName) {
	if (!_serverSocket || !_loggedIn) {
		cout << "you are not connected or not logged in" << endl;
		return;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, LIST_CONNECTED_USERS_IN_ROOM);
	MessengerClient::sendDataToPeer(_serverSocket, roomName);
}

// delete the room by it's name (only the owner of the room can delete it)
bool MessengerClient::deleteChatRoom(string name) {
	if (!_serverSocket || !_loggedIn) {
		cout << "you are not connected/logged in" << endl;
		return false;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, CHAT_ROOM_CLOSE);
	MessengerClient::sendDataToPeer(_serverSocket, name);

	return true;
}

// disconnect the open session / exit from a room
bool MessengerClient::exitRoomOrCloseSession() {
	if (_roomOn) {
		MessengerClient::sendCommandToPeer(_serverSocket, CHAT_ROOM_EXIT);
	} else if (_sessionOn) {
		MessengerClient::sendCommandToPeer(_serverSocket, SESSION_CLOSE);
	} else {
		return false;
	}

	return true;

}

// adds room mate vector of the users
void MessengerClient::addRoomUser(string roomate, string IP, int port) {
	ChatSideB *temp = new ChatSideB();
	temp->_sideB_name = roomate;
	temp->_IP = IP;
	temp->_port = port;
	_chatUsers.push_back(temp);
}

// update the user details after leaving a room
void MessengerClient::chatRoomLeaved() {
	_roomOn = false;
	clearRoomUsers();
	delete (_clientLinker);
}

// clear the vector of the users - (after exiting a room)
void MessengerClient::clearRoomUsers() {
	std::vector<ChatSideB*>::iterator iter = _chatUsers.begin();
	std::vector<ChatSideB*>::iterator enditer = _chatUsers.end();

	while (iter != enditer) {
		delete (*iter);
		iter++;
	}

	_chatUsers.clear();
}

// return true if the user is in a session or in a chat. false otherwise
bool MessengerClient::isInChat() {
	return _roomOn || _sessionOn;
}

bool MessengerClient::isConnectedToServer() const {
	return _connectedToServer;
}

// disconnect from the server and exit
void MessengerClient::exitAll() {
	exitRoomOrCloseSession();

	MessengerClient::sendCommandToPeer(_serverSocket, EXIT);

	_username.clear();
	_running = false;
	_loggedIn = false;
	_connectedToServer = false;
	_serverSocket = NULL;
}

// print the list rooms
void MessengerClient::printRoomsList() {
	int numOfRooms;
	numOfRooms = MessengerClient::readCommandFromPeer(_serverSocket);

	if (numOfRooms > 0) {
		cout << "Rooms: " << endl;
		for (int i = 0; i < numOfRooms; i++) {
			cout << "Room name: " << MessengerClient::readDataFromPeer(_serverSocket) << endl;
		}
	} else
		cout << "There are no rooms yet" << endl;
}

// update the rooms info
void MessengerClient::chatRoomUpdate() {
	_roomOn = true;
	clearRoomUsers();
	_chatRoomName = MessengerClient::readDataFromPeer(_serverSocket);
	int countofmemebers = MessengerClient::readCommandFromPeer(_serverSocket);
	for (int i = 0; i < countofmemebers; i++) {
		string user = MessengerClient::readDataFromPeer(_serverSocket);
		string ip = MessengerClient::readDataFromPeer(_serverSocket);
		int port = MessengerClient::readCommandFromPeer(_serverSocket);
		addRoomUser(user, ip, port);
	}

	cout << "Chat room " << _chatRoomName << " updated" << endl;
}
