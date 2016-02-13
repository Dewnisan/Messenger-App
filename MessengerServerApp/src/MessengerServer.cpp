#include <string>

#include "MessengerServer.h"
#include "MultipleTCPSocketsListener.h"
#include "TCPMessengerProtocolExtentions.h"

using namespace std;

void MessengerServer::createSession(User* fromUser, User* toUser) {
	// login the two users
	toUser->loginUsertoSession(fromUser);
	fromUser->loginUsertoSession(toUser);

	// send communication details
	toUser->writeCommand(SESSION_ESTABLISHED);
	toUser->writeMsg(fromUser->getusername());
	toUser->writeMsg(fromUser->getIP());
	toUser->writeCommand(fromUser->getport());
	toUser->writeCommand(toUser->getport());

	fromUser->writeCommand(SESSION_ESTABLISHED);
	fromUser->writeMsg(toUser->getusername());
	fromUser->writeMsg(toUser->getIP());
	fromUser->writeCommand(toUser->getport());
	fromUser->writeCommand(fromUser->getport());
}

MessengerServer::MessengerServer(const std::string& pathToUsersFile) :
		_running(false), _pathToUsersFile(pathToUsersFile) {

	start();
	cout << "Messenger server is up" << endl;
}

MessengerServer::~MessengerServer() {
	_running = false;
	waitForThread();
}

void MessengerServer::run() {
	_running = true;

	while (_running) {
		MultipleTCPSocketsListener multipleSocketsListener;

		// Convert User to socket before adding to list
		vector<TCPSocket*> sockets;
		for (map<string, User*>::iterator iter = _users.begin();
				iter != _users.end(); iter++) {
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
		for (map<string, User*>::iterator iter = _users.begin();
				iter != _users.end(); iter++) {
			if (readySock == iter->second->getSocket()) {
				currUser = iter->second;
				break;
			}
		}

		string msg;
		// Read command from the user
		int command = currUser->readCommand();
		switch (command) {
		case 0:
			exitServer(currUser);
			break;
		case SESSION_CREATE:
			msg = currUser->readMsg(); // the partner name
			if (!_users[msg]) {
				currUser->writeCommand(SESSION_CREATE_REFUSED);
				currUser->writeMsg(string("there is no such user"));
				break;
			} else if (_users[msg]->inChat()) {
				currUser->writeCommand(SESSION_CREATE_REFUSED);
				currUser->writeMsg(string("the wanted user is in chat"));
				break;
			}
			createSession(currUser, _users[msg]);
			cout << "Session was created between: " << currUser->getusername()
					<< " AND " << msg << endl;
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
			loginChatRoom(currUser);
			break;
		case CHAT_ROOM_EXIT:
			currUser->disconnectFromChatRom(false);
			break;
		case LIST_CONNECTED_USERS:
			getListConnectedUsers(currUser);
			break;
		case LIST_CHAT_ROOMS:
			getListRooms(currUser);
			break;
		case LIST_CONNECTED_USERS_IN_CHAT_ROOM:
			getListChatUsers(currUser);
			break;
		case LIST_USERS:
			getListUsers(currUser);
			break;
		}
	}
}

// add user to users map
bool MessengerServer::addUser(TCPSocket* userSocket, string LoginUserName) {
	bool added = true;
	User* userToAdd = new User(LoginUserName, userSocket);

	_users.insert(pair<string, User*>(LoginUserName, userToAdd));
	cout << LoginUserName << " has logged in" << endl;

	return added;
}

// Print List of the connected users
int MessengerServer::listConnectedUsers() {
	if (_users.begin() == _users.end()) {
		cout << "There aren't any users connected" << endl;
		return 0;
	}

	cout << "The Connected users are:" << endl;

	map<string, User*>::iterator iter;

	string name;
	int count = 0;
	for (iter = _users.begin(); iter != _users.end(); iter++) {
		name = iter->first;
		printToSreen(name);
		count++;
	}
	return count;
}

// Send list of the connected users to the asking client
int MessengerServer::getListConnectedUsers(User *client) {
	client->writeCommand(LIST_CONNECTED_USERS);
	client->writeCommand(_users.size());

	map<string, User*>::iterator iter;

	string name;
	int count = 0;
	for (iter = _users.begin(); iter != _users.end(); iter++) {
		name = iter->first;
		client->writeMsg(name);
		count++;
	}

	return count;
}

bool MessengerServer::isConnected(string username) {
	if (_users.find(username) == _users.end())
		return false; // not found
	return true;
}

// get list of all open sessions
void MessengerServer::listSessions() {
	cout << "All the connected users that in Session:" << endl;
	std::map<string, User*>::iterator iter;

	string name;
	for (iter = _users.begin(); iter != _users.end(); iter++) {
		if (iter->second->inSession()) {
			name = iter->first;
			printToSreen(name);
		}
	}
}

// Print list of all chat rooms
void MessengerServer::listRooms() {
	if (_chatRooms.begin() != _chatRooms.end()) {
		cout << "the rooms list:" << endl;
		this->readfromChatRoom(NULL);
	} else
		cout << "There are no rooms yet" << endl;
}

// Send  list of chat rooms to the asking client
void MessengerServer::getListRooms(User *client) {
	client->writeCommand(LIST_CHAT_ROOMS);
	client->writeCommand(_chatRooms.size());

	this->readfromChatRoom(client);
}

// Print list of users in a specific room
int MessengerServer::listChatUsers(string ChatRoomName) {
	int numOfUsers;
	if (_chatRooms.find(ChatRoomName) == _chatRooms.end()) {
		cout << "No such room: " << ChatRoomName << endl;
		return 0;
	}
	cout << "Users list in Room:" << endl;
	for (map<string, ChatRoom*>::iterator iter = _chatRooms.begin();
			iter != _chatRooms.end(); iter++) {
		if (ChatRoomName == iter->first) {
			numOfUsers = iter->second->printUsers();
		}
	}

	if (numOfUsers == 0)
		cout << "There are no users in this room" << endl;

	return numOfUsers;
}

// Send list of users in a specific room to the asking client
int MessengerServer::getListChatUsers(User *client) {
	int numOfUsers = 0;
	string ChatRoomName = client->readMsg();
	for (map<string, ChatRoom*>::iterator iter = _chatRooms.begin();
			iter != _chatRooms.end(); iter++) {
		if (ChatRoomName == iter->first) {
			client->writeCommand(LIST_CONNECTED_USERS_IN_CHAT_ROOM);
			(iter)->second->sendUserList(client);
			break;
		}
	}
	return numOfUsers;
}

// Send List of users from login file to asking client
void MessengerServer::getListUsers(User *client) {
	int numOfusers = 0;
	numOfusers = numOfUsersFromFile();

	client->writeCommand(LIST_USERS);
	client->writeCommand(numOfusers - 1);

	if (client != NULL)
		readFromFile(client);
	else
		readFromFile(NULL);

}

// Print List of users from login file
void MessengerServer::listUsers() {
	cout << "the users:" << endl;
	readFromFile(NULL); // To server
}

// user exit from the server
void MessengerServer::exitServer(User* clientName) {
	//clientName->closeSession(true);
	//clientName->disconnectFromChatRom(false);

	cout << "the User: " << clientName->getusername() << " was  disconnect"
			<< endl;
	_users.erase(clientName->getusername());
}

// Create new chat room
void MessengerServer::createChatRoom(User* creator) {
	string msg;
	bool exist = false;
	msg = creator->readMsg();

	// Checking that the name does not exist
	for (map<string, ChatRoom*>::iterator iter = _chatRooms.begin();
			iter != _chatRooms.end(); iter++) {
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

// delete chat room
void MessengerServer::deleteChatRoom(User* creator) {
	string msg;
	msg = creator->readMsg();

	bool exist = false;

	for (map<string, ChatRoom*>::iterator iter = _chatRooms.begin();
			iter != _chatRooms.end(); iter++) {
		if (iter->first == msg) {
			exist = true;
		}
	}

	if (!exist) {
		creator->writeCommand(CHAT_ROOM_UNCLOSED);
	}

	if (_chatRooms[msg]->getOwner()->getusername() == creator->getusername()) {
		delete (_chatRooms[msg]);
		creator->writeCommand(CHAT_ROOM_CLOSED);
		_chatRooms.erase(msg);
	} else {
		creator->writeCommand(CHAT_ROOM_UNCLOSED);
	}

}

// login to chat room
void MessengerServer::loginChatRoom(User* loginUser) {
	string roomName = loginUser->readMsg();
	bool exist = false;
	for (map<string, ChatRoom*>::iterator iter = _chatRooms.begin();
			iter != _chatRooms.end(); iter++) {
		if (iter->first == roomName) {
			exist = true;
		}
	}

	if (!exist) {
		loginUser->writeCommand(CHAT_ROOM_ENTERING_DENIED);
		loginUser->writeMsg(string("Room does not exist"));
		return;
	}

	loginUser->loginUserToChatRoom(_chatRooms[roomName]);
	if (_chatRooms[roomName]->addUser(loginUser)) // addUser of ChatRoom
			{
		loginUser->writeCommand(CHAT_ROOM_USER_ENTERED);
		loginUser->writeCommand(loginUser->getport());
	} else {
		loginUser->writeCommand(CHAT_ROOM_ENTERING_DENIED);
		loginUser->writeMsg(string("you are already logged in"));
	}
}

// returns the number of registered users
int MessengerServer::numOfUsersFromFile() {
	string line;
	fstream loginFile;
	string userFromFile;
	int counter = 0;

	loginFile.open(_pathToUsersFile.c_str(), ios::in | ios::out | ios::binary);

	while (!loginFile.eof()) {
		getline(loginFile, line);
		counter++;
	}
	loginFile.close();
	return counter;
}

// print/send all the users that are registered (in the file)
void MessengerServer::readFromFile(User *clientName) {
	string line;
	fstream loginFile;
	string userFromFile;
	loginFile.open(_pathToUsersFile.c_str(), ios::in | ios::out | ios::binary);
	if (loginFile.is_open()) {
		while (!loginFile.eof()) {
			getline(loginFile, line);
			istringstream liness(line);
			getline(liness, userFromFile, '-');
			if (clientName != NULL) {
				clientName->writeMsg(userFromFile); // send to client who requested
			} else {
				printToSreen(userFromFile); // Print on server
			}

		}
		loginFile.close();
	} else {
		printToSreen("Error - could not open the file");
	}

}

void MessengerServer::printToSreen(string msgToScreen) {
	cout << msgToScreen << endl;
}

// print/send all the rooms
void MessengerServer::readfromChatRoom(User *clientName) {
	string name;
	for (map<string, ChatRoom*>::iterator iter = _chatRooms.begin();
			iter != _chatRooms.end(); iter++) {
		name = iter->first;
		if (clientName != NULL) {
			clientName->writeMsg(name); // send to client who requested
		} else {
			printToSreen(name); // Print on server
		}

	}
}
