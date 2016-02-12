#ifndef MESSEMGERCLIENT_H_
#define MESSEMGERCLIENT_H_

#include <string>
#include <vector>

#include "MThread.h"
#include "ClientLinker.h"
#include "MessengerEntity.h"

#include "TCPMessengerProtocol.h"
#include "TCPSocket.h"

class ChatRemoteSide {
	std::string _name;
	std::string _ip;
	int _port;

public:
	ChatRemoteSide(std::string name, std::string ip, int port) :
			_name(name), _ip(ip), _port(port) {
	}

	ChatRemoteSide() :
			_port(0) {
	}

	virtual ~ChatRemoteSide() {
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

class MessengerClient: public MThread, MessengerEntity {
	bool _running;
	TCPSocket* _serverSocket;
	ChatRemoteSide _udpChatSideB;
	string _username;
	vector<ChatRemoteSide*> _chatUsers;
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

	// Adds room mate vector of the users
	void addRoomUser(string name, string ip, int port);

public:
	MessengerClient();
	virtual ~MessengerClient();

	// Connects to the given server IP and port
	bool connectToServer(string ip, int port);

	// Registers a new user to server with a given user name and password
	void signup(string username, string password);

	// Logins to server with a given user name and password
	void login(string username, string password);

	// Send to the server request of list of the users from the file
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

	// Disconnects the open session OR exit from a room
	bool closeSessionOrExitRoom();

	// Prints the current status of the client
	void printCurrentInfo();

	// Disconnects from the server and exits
	void disconnectFromServer();

	bool connectedToServer() const;

	// Returns true if the user is in a session or in a chat, false otherwise
	bool isConversing() const;

	// Prints all of the connected users after received from the server
	void printConnectedUsers();

	// Print the list of the users from the file
	void printListUsers();

	// Updates the user details after leaving a room
	void chatRoomLeaved();

	// Updates the rooms info
	void chatRoomUpdate();

	// Prints the list rooms
	void printRoomsList();
};

#endif /* MESSEMGERCLIENT_H_ */
