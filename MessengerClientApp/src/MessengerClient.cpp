#include "MessengerClient.h"
#include "TCPSocket.h"
#include "TCPMessengerProtocolExtentions.h"

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
			_inSession = true;

			cout << "You are in direct connection with  " << _udpChatSideB._sideB_name << endl;
			break;
		case SESSION_CREATE_REFUSED:
			parameter1 = MessengerClient::readDataFromPeer(_serverSocket);
			cout << "Session has been denied because " << parameter1 << endl;
			break;
		case SESSION_CLOSED:
			_udpChatSideB.clean();
			_inSession = false;
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
			_inChatRoom = true;
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

void MessengerClient::clearRoomUsers() {
	std::vector<ChatSideB*>::iterator iter = _chatUsers.begin();
	std::vector<ChatSideB*>::iterator enditer = _chatUsers.end();

	while (iter != enditer) {
		delete (*iter);
		iter++;
	}

	_chatUsers.clear();
}

void MessengerClient::addRoomUser(string roomate, string IP, int port) {
	ChatSideB *temp = new ChatSideB();
	temp->_sideB_name = roomate;
	temp->_IP = IP;
	temp->_port = port;
	_chatUsers.push_back(temp);
}

MessengerClient::MessengerClient() :
		_running(false), _serverSocket(NULL), _clientLinker(NULL), _connected(false),
		_loggedIn(false), _inSession(false), _inChatRoom(false) {
}

MessengerClient::~MessengerClient() {
	_running = false;
}

bool MessengerClient::connectToServer(string ip, int port) {
	if (_serverSocket != NULL) {
		return false;
	}

	_serverSocket = new TCPSocket(ip, port);
	_connected = true;

	return true;
}

void MessengerClient::signup(string username, string password) {
	if (_loggedIn) {
		cout << "Cannot register while logged in" << endl;
		return;
	}

	if (_connected) {
		MessengerClient::sendCommandToPeer(_serverSocket, REGISTRATION_REQUEST);
		MessengerClient::sendDataToPeer(_serverSocket, username);
		MessengerClient::sendDataToPeer(_serverSocket, password);

		int response = MessengerClient::readCommandFromPeer(_serverSocket);
		if (response == REGISTRATION_REQUEST_APPROVED) {
			cout << "Signed up was successful with the user: " << username << endl;
		} else if (response == REGISTRATION_REQUEST_DENIED) {
			cout << "You have failed to sign up" << endl;
		}
	} else {
		cout << "Not connected to server" << endl;
	}
}

void MessengerClient::login(string username, string password) {
	if (_loggedIn) {
		cout << "Already logged in" << endl;
		return;
	}

	if (_connected) {
		MessengerClient::sendCommandToPeer(_serverSocket, LOGIN_REQUEST);
		MessengerClient::sendDataToPeer(_serverSocket, username);
		MessengerClient::sendDataToPeer(_serverSocket, password);

		int response = MessengerClient::readCommandFromPeer(_serverSocket);
		if (response == LOGIN_REQUEST_APPROVED) {
			_username = username;
			_loggedIn = true;

			start();

			cout << "Logged in as " + username << endl;
		} else if (response == LOGIN_REQUEST_WRONG_DETAILS) {
			cout << "Wrong user name or password" << endl;
		} else if (response == LOGIN_REQUEST_ALREADY_LOGGED) {
			cout << username + " already logged in" << endl;
		}
	} else {
		cout << "Not connected to server" << endl;
	}
}

void MessengerClient::listUsers() {
	if (!_loggedIn) {
		cout << "You are not logged in" << endl;
		return;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, LIST_USERS);
}

void MessengerClient::listConnectedUsers() {
	if (!_loggedIn) {
		cout << "You are not logged in" << endl;
		return;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, LIST_CONNECTED_USERS);
}

bool MessengerClient::createChatRoom(string roomName) {
	if (!_loggedIn || isConversing()) {
		return false;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, CHAT_ROOM_CREATE);
	MessengerClient::sendDataToPeer(_serverSocket, roomName);

	return true;
}

bool MessengerClient::enterChatRoom(string roomName) {
	if (!_loggedIn || isConversing()) {
		return false;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, CHAT_ROOM_LOGIN);
	MessengerClient::sendDataToPeer(_serverSocket, roomName);

	return true;
}

bool MessengerClient::deleteChatRoom(string name) {
	if (!_loggedIn) {
		cout << "you are not logged in" << endl;
		return false;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, CHAT_ROOM_CLOSE);
	MessengerClient::sendDataToPeer(_serverSocket, name);

	return true;
}

void MessengerClient::listRooms() {
	if (!_loggedIn) {
		cout << "You are not logged in" << endl;
		return;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, LIST_ROOMS);
}

void MessengerClient::listRoomUsers(string roomName) {
	if (!_loggedIn) {
		cout << "You are not logged in" << endl;
		return;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, LIST_CONNECTED_USERS_IN_ROOM);
	MessengerClient::sendDataToPeer(_serverSocket, roomName);
}

bool MessengerClient::openSession(string peerName) {
	if (!_loggedIn) {
		cout << "Cannot open session without being logged in" << endl;
		return false;
	}

	if (peerName == _username) {
		cout << "Cannot open session with yourself" << endl;
		return false;
	}

	if (isConversing()) {
		cout << "Already in a session" << endl;
		return false;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, SESSION_CREATE);
	MessengerClient::sendDataToPeer(_serverSocket, peerName);

	return true;
}

bool MessengerClient::sendMessage(string msg) {
	if (_inSession) {
		_clientLinker->send(">[" + _username + "]" + msg, _udpChatSideB._IP, _udpChatSideB._port);

		return true;
	} else if (_inChatRoom) {
		std::vector<ChatSideB*>::iterator iter = _chatUsers.begin();
		std::vector<ChatSideB*>::iterator enditer = _chatUsers.end();

		while (iter != enditer) {
			_clientLinker->send(string(">[") + _username + string("] ") + msg, (*iter)->_IP, (*iter)->_port);
			iter++;
		}

		return true;
	} else {
		cout << "You need to create a session or login to a room first" << endl;
	}

	return false;
}

bool MessengerClient::closeSessionOrExitRoom() {
	if (_inChatRoom) {
		MessengerClient::sendCommandToPeer(_serverSocket, CHAT_ROOM_EXIT);
	} else if (_inSession) {
		MessengerClient::sendCommandToPeer(_serverSocket, SESSION_CLOSE);
	} else {
		return false;
	}

	return true;
}

void MessengerClient::printCurrentInfo() {
	if (_serverSocket != NULL) {
		cout << "Connected to server " << endl;
	} else {
		cout << "Not Connected to server " << endl;
	}

	if (_loggedIn) {
		cout << "Logged in as  " << _username << endl;
	} else {
		cout << "Not logged in " << endl;
	}

	if (_inSession) {
		cout << "In session " << endl;
	} else {
		cout << "Not in session " << endl;
	}

	if (_inChatRoom) {
		cout << "In chat room: " << _chatRoomName << endl;
	} else {
		cout << "Not in a chat room " << endl;
	}
}

void MessengerClient::exitAll() {
	closeSessionOrExitRoom();

	MessengerClient::sendCommandToPeer(_serverSocket, EXIT);

	_username.clear();
	_running = false;
	_loggedIn = false;
	_connected = false;
	_serverSocket = NULL;
}

bool MessengerClient::connectedToServer() const {
	return _connected;
}

void MessengerClient::printConnectedUsers() {
	int numOfUsers = MessengerClient::readCommandFromPeer(_serverSocket);
	for (int i = 0; i < numOfUsers; i++) {
		cout << "User: " << MessengerClient::readDataFromPeer(_serverSocket) << endl;
	}

	if (numOfUsers == 0) {
		cout << "No one is connected" << endl;
	}
}

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

void MessengerClient::chatRoomLeaved() {
	_inChatRoom = false;
	clearRoomUsers();
	delete (_clientLinker);
}

bool MessengerClient::isConversing() const {
	return _inChatRoom || _inSession;
}

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

void MessengerClient::chatRoomUpdate() {
	_inChatRoom = true;
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
