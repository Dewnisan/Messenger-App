#ifndef CLIENTMANAGER_H_
#define CLIENTMANAGER_H_

#include <vector>

#include "MThread.h"
#include "ClientLinker.h"
#include "MessengerEntity.h"

#include "TCPMessengerProtocol.h"
#include "TCPSocket.h"

class ChatSideB {
public:
	string _sideB_name;
	string _IP;
	int _port;

	void clean() {
		_sideB_name.clear();
		_IP.clear();
		_port = 0;
	}
};

class MessengerClient: public MThread, MessengerEntity {
	bool _running;
	TCPSocket* _serverSocket;
	ChatSideB _udpChatSideB;
	string _username;
	vector<ChatSideB*> _chatUsers;
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
	void addRoomUser(string roomate, string IP, int port);

public:
	MessengerClient();
	virtual ~MessengerClient();

	// Connects to the given server IP and port
	bool connectToServer(string ip, int port);

	bool isConnectedToServer() const;

	// Registers a new user to server with a given user name and password
	void signup(string username, string password);

	// Logins to server with a given user name and password
	void login(string username, string password);

	// Opens session with the given peer name
	bool openSession(string peerBName);

	// Prints the current status of the client
	void printCurrentInfo();

	// Sends a UDP message to specific user or to all the users in a chat room
	bool sendMessage(string msg);

	// Opens session in the given room name
	bool createChatRoom(string name);

	// Logins to a room by it's room name
	bool loginToChatRoom(string roomName);

	// Sends to the server request of connected users
	void printConnectedUsersRequest();

	// Prints all of the connected users after received from the server
	void printConnectedUsers();

	// Print the list of the users from the file
	void printListUsers();

	// Sends to the server request of room list
	void roomsList();

	// Sends to the server request of all the users in a room (given by it's name)
	void listConnectedUsersInRoom(string roomName);

	// Send to the server request of list of the users from the file
	void listUsers();

	// Disconnects the open session OR exit from a room
	bool exitRoomOrCloseSession();

	// Deletes the room by it's name (only the owner of the room can delete it)
	bool deleteChatRoom(string msg);

	// Returns true if the user is in a session or in a chat, false otherwise
	bool isConversing();

	// Disconnects from the server and exits
	void exitAll();

	// Updates the user details after leaving a room
	void chatRoomLeaved();

	// Updates the rooms info
	void chatRoomUpdate();

	// Prints the list rooms
	void printRoomsList();
};

#endif /* CLIENTMANAGER_H_ */
