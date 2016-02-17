#ifndef MESSEMGERCLIENT_H_
#define MESSEMGERCLIENT_H_

#include <string>
#include <vector>

#include "MThread.h"
#include "ClientLinker.h"
#include "MessengerEntity.h"

#include "TCPMessengerProtocol.h"
#include "TCPSocket.h"

class Peer {
	std::string _name;
	std::string _ip;
	int _port;

public:
	Peer(std::string name, std::string ip, int port) :
			_name(name), _ip(ip), _port(port) {
	}

	Peer() :
			_port(0) {
	}

	virtual ~Peer() {
	}

	void reset() {
		_name.clear();
		_ip.clear();
		_port = 0;
	}

	const std::string& getIp() const {
		return _ip;
	}

	void setIp(const std::string& ip) {
		_ip = ip;
	}

	const std::string& getName() const {
		return _name;
	}

	void setName(const std::string& name) {
		_name = name;
	}

	int getPort() const {
		return _port;
	}

	void setPort(int port) {
		_port = port;
	}
};

class MessengerClient: public MThread, public MessengerEntity {
	bool _running;
	string _username;

	TCPSocket* _serverSocket;

	Peer _sessionPeer; // Used when opening a session with one user

	vector<Peer*> _chatUsers; // Used when entering a chat room
	string _chatRoomName;

	ClientLinker *_clientLinker;

	bool _connected;
	bool _loggedIn;
	bool _inSession;
	bool _inChatRoom;

	// Receive loop, activated when logging into the server
	void run();

	// Clears the pool of the users (after exiting a room)
	void clearRoomUsers();

	// Adds user to pool of the chat room users
	void addChatRoomUser(string name, string ip, int port);

	// Updates the user's details after leaving a chat room
	void chatRoomLeft();

	// Updates the rooms info
	void chatRoomUpdate();

	// Print the list of the users according to server's response
	void printListUsers();

	// Prints all of the connected users according to server's response
	void printConnectedUsers();

	// Prints the list of chat rooms according to server's response
	void printRoomsList();

public:
	MessengerClient();
	virtual ~MessengerClient();

	// Connects to the given server IP and port
	bool connectToServer(string ip, int port);

	// Registers a new user to server with a given user name and password
	void signup(string username, string password);

	// Logins to server with a given user name and password
	void login(string username, string password);

	// Sends to the server request of list of the users from the file
	void listUsers();

	// Sends to the server request of connected users
	void listConnectedUsers();

	// Opens session in the given room name
	bool createChatRoom(string roomName);

	// Enters to a room by room's name
	bool enterChatRoom(string roomName);

	// Deletes the room by it's name (only the owner of the room can delete it)
	bool deleteChatRoom(string msg);

	// Sends to the server request of room list
	void listRooms();

	// Sends to the server request of all the users in a room (given by it's name)
	void listRoomUsers(string roomName);

	// Opens session with the given peer name
	bool openSession(string peerBName);

	// Sends a UDP message to specific user or to all the users in a chat room
	bool sendMessage(string msg);

	// Disconnects the open session OR exit from a chat room
	bool closeSessionOrExitChatRoom();

	// Prints the current status of the client
	void printCurrentInfo();

	// Disconnects from the server and exits
	void disconnectFromServer();

	bool connectedToServer() const;

	// Returns true if the user is in a session or in a chat, false otherwise
	bool isConversing() const;
};

#endif /* MESSEMGERCLIENT_H_ */
