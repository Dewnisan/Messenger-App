#include "MessengerClient.h"
#include "TCPSocket.h"
#include "TCPMessengerProtocolExtentions.h"

void MessengerClient::run() {
	string parameter1;

	_running = true;
	while (_running) {
		int command = MessengerClient::readCommandFromPeer(_serverSocket);
		if (command == 0) {
			continue;
		}

		string name;
		string ip;
		int port;
		int partnerPort;

		switch (command) {
		case SESSION_ESTABLISHED:
			name = MessengerClient::readDataFromPeer(_serverSocket);
			ip = MessengerClient::readDataFromPeer(_serverSocket);
			port = MessengerClient::readCommandFromPeer(_serverSocket);

			_sessionPeer.setName(name);
			_sessionPeer.setIp(ip);
			_sessionPeer.setPort(port);

			partnerPort = MessengerClient::readCommandFromPeer(_serverSocket);
			_clientLinker = new ClientLinker(partnerPort);
			_inSession = true;

			cout << "You are in direct connection with  " << _sessionPeer.getName() << endl;
			break;
		case SESSION_CREATE_REFUSED:
			parameter1 = MessengerClient::readDataFromPeer(_serverSocket);
			cout << "Session has been denied because " << parameter1 << endl;
			break;
		case SESSION_CLOSED:
			_sessionPeer.reset();
			_inSession = false;

			delete (_clientLinker);
			_clientLinker = NULL;

			cout << "Session is now terminated" << endl;
			break;
		case CHAT_ROOM_CREATE_DENIED:
			parameter1 = MessengerClient::readDataFromPeer(_serverSocket);
			cout << "Room cannot be opened because " << parameter1 << endl;
			break;
		case CHAT_ROOM_CREATED:
			cout << "Chat room has been opened" << endl;
			break;
		case CHAT_ROOM_USER_LEFT:
			chatRoomLeft();
			cout << "Your user left the chat room" << endl;
			break;
		case CHAT_ROOM_USER_ENTERED:
			partnerPort = MessengerClient::readCommandFromPeer(_serverSocket); // UDP listen port
			_clientLinker = new ClientLinker(partnerPort);
			_inChatRoom = true;

			cout << "You have joined to the room" << endl;
			break;
		case CHAT_ROOM_ENTERING_DENIED:
			cout << MessengerClient::readDataFromPeer(_serverSocket) << endl;
			break;
		case CHAT_ROOM_UPDATED:
			chatRoomUpdate();
			break;
		case CHAT_ROOM_CLOSED:
			cout << "The chat room has been closed" << endl;
			break;
		case CHAT_ROOM_UNCLOSED:
			cout << "You cannot delete the chat room" << endl;
			break;
		case CHAT_ROOM_NOT_EXIST:
			cout << "Chat room does not exist" << endl;
			break;
		case LIST_CONNECTED_USERS:
			printConnectedUsers();
			break;
		case LIST_CHAT_ROOMS:
			printRoomsList();
			break;
		case LIST_CONNECTED_USERS_IN_CHAT_ROOM:
			printConnectedUsers(); // Fits to our need
			break;
		case LIST_USERS:
			printListUsers();
			break;
		default:
			cout << "Unrecognized command received from server" << endl;
			break;
		}
	}
}

void MessengerClient::clearRoomUsers() {
	for (vector<Peer*>::iterator iter = _chatUsers.begin(); iter != _chatUsers.end(); iter++) {
		delete *iter;
	}

	_chatUsers.clear();
}

void MessengerClient::addChatRoomUser(string name, string ip, int port) {
	Peer *temp = new Peer(name, ip, port);
	_chatUsers.push_back(temp);
}

void MessengerClient::chatRoomLeft() {
	_inChatRoom = false;
	clearRoomUsers();

	delete _clientLinker;
	_clientLinker = NULL;
}

void MessengerClient::chatRoomUpdate() {
	_inChatRoom = true;

	clearRoomUsers();
	_chatRoomName = MessengerClient::readDataFromPeer(_serverSocket);

	int numOfMemebers = MessengerClient::readCommandFromPeer(_serverSocket);
	for (int i = 0; i < numOfMemebers; i++) {
		string user = MessengerClient::readDataFromPeer(_serverSocket);
		string ip = MessengerClient::readDataFromPeer(_serverSocket);
		int port = MessengerClient::readCommandFromPeer(_serverSocket);
		addChatRoomUser(user, ip, port);
	}

	cout << "Chat room " << _chatRoomName << " updated" << endl;
}

void MessengerClient::printListUsers() {
	int numOfUsers = MessengerClient::readCommandFromPeer(_serverSocket);
	if (numOfUsers > 0) {
		for (int i = 0; i < numOfUsers; i++) {
			cout << " user: " << MessengerClient::readDataFromPeer(_serverSocket) << endl;
		}
	} else {
		cout << "No one is connected yet" << endl;
	}
}

void MessengerClient::printConnectedUsers() {
	int numOfUsers = MessengerClient::readCommandFromPeer(_serverSocket);
	if (numOfUsers > 0) {
		for (int i = 0; i < numOfUsers; i++) {
			cout << "User: " << MessengerClient::readDataFromPeer(_serverSocket) << endl;
		}
	} else {
		cout << "No one is connected yet" << endl;
	}
}

void MessengerClient::printRoomsList() {
	int numOfRooms = MessengerClient::readCommandFromPeer(_serverSocket);
	if (numOfRooms > 0) {
		cout << "Rooms: " << endl;
		for (int i = 0; i < numOfRooms; i++) {
			cout << "Room name: " << MessengerClient::readDataFromPeer(_serverSocket) << endl;
		}
	} else {
		cout << "There are no chat rooms yet" << endl;
	}
}

MessengerClient::MessengerClient() :
		_running(false), _serverSocket(NULL), _clientLinker(NULL), _connected(false),
		_loggedIn(false), _inSession(false), _inChatRoom(false) {
}

MessengerClient::~MessengerClient() {
	_running = false;

	if (_serverSocket != NULL) {
		delete _serverSocket;
		_serverSocket = NULL;
	}

	if (_clientLinker != NULL) {
		delete _clientLinker;
		_clientLinker = NULL;
	}
}

bool MessengerClient::connectToServer(string ip, int port) {
	if (_serverSocket != NULL) {
		cout << "Connection failed - already connected" << endl;
		return false;
	}

	_serverSocket = new TCPSocket(ip, port);
	_connected = true;

	cout << "Connected to: " << ip << endl;

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
			cout << "Successfully registered the user: " << username << endl;
		} else if (response == REGISTRATION_REQUEST_DENIED) {
			cout << "Registration failed" << endl;
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
	if (!_loggedIn) {
		cout << "You are not logged in" << endl;
		return false;
	} else if (isConversing()) {
		cout << "Cannot perform action while user is conversing" << endl;
		return false;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, CHAT_ROOM_CREATE);
	MessengerClient::sendDataToPeer(_serverSocket, roomName);

	return true;
}

bool MessengerClient::enterChatRoom(string roomName) {
	if (!_loggedIn) {
		cout << "You are not logged in" << endl;
		return false;
	}

	if (isConversing()) {
		closeSessionOrExitChatRoom();
	}

	MessengerClient::sendCommandToPeer(_serverSocket, CHAT_ROOM_ENTER);
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

	MessengerClient::sendCommandToPeer(_serverSocket, LIST_CHAT_ROOMS);
}

void MessengerClient::listRoomUsers(string roomName) {
	if (!_loggedIn) {
		cout << "You are not logged in" << endl;
		return;
	}

	MessengerClient::sendCommandToPeer(_serverSocket, LIST_CONNECTED_USERS_IN_CHAT_ROOM);
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
		closeSessionOrExitChatRoom();
	}

	MessengerClient::sendCommandToPeer(_serverSocket, SESSION_CREATE);
	MessengerClient::sendDataToPeer(_serverSocket, peerName);

	return true;
}

bool MessengerClient::sendMessage(string msg) {
	if (_inSession) {
		_clientLinker->send(">[" + _username + "]" + msg, _sessionPeer.getIp(), _sessionPeer.getPort());

		return true;
	} else if (_inChatRoom) {
		for (vector<Peer*>::iterator iter = _chatUsers.begin(); iter != _chatUsers.end(); iter++) {
			_clientLinker->send(string(">[") + _username + string("] ") + msg, (*iter)->getIp(), (*iter)->getPort());
		}

		return true;
	} else {
		cout << "You need to create a session or login to a room first" << endl;
	}

	return false;
}

bool MessengerClient::closeSessionOrExitChatRoom() {
	if (_inChatRoom) {
		MessengerClient::sendCommandToPeer(_serverSocket, CHAT_ROOM_EXIT);
		_inChatRoom = false;
		cout << "Exited from chat room " + _chatRoomName << endl;
	} else if (_inSession) {
		MessengerClient::sendCommandToPeer(_serverSocket, SESSION_CLOSE);
		_inSession = false;
		cout << "Closed session with " + _sessionPeer.getName() << endl;
	} else {
		cout << "There is not session or room to exit from" << endl;
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

void MessengerClient::disconnectFromServer() {
	closeSessionOrExitChatRoom();

	MessengerClient::sendCommandToPeer(_serverSocket, EXIT);

	_username.clear();

	_running = false;
	_loggedIn = false;
	_connected = false;

	_serverSocket->cclose();
	delete _serverSocket;
	_serverSocket = NULL;
}

bool MessengerClient::connectedToServer() const {
	return _connected;
}

bool MessengerClient::isConversing() const {
	return _inChatRoom || _inSession;
}
