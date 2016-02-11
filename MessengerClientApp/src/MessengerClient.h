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

	TCPSocket* _serverSocket;
	bool _running;
	ChatSideB _udpChatSideB;
	string _username;
	vector<ChatSideB*> _chatUsers;
	string _chatRoomName;
	ClientLinker *_clientLinker;
	bool _sessionOn;
	bool _roomOn;
	bool _loggedIn;
	bool _connectedToServer;

	// client receive loop, activated once a login to the server is established
	void run();

	// clear the vector of the users - (after exiting a room)
	void clearRoomUsers();

	// adds room mate vector of the users
	void addRoomUser(string roomate, string IP, int port);

public:
	MessengerClient();
	virtual ~MessengerClient();

	// Connect to the given server ip and port
	bool connectToServer(string ip, int port);

	// Register a new user to server with a given username and password
	void signup(string username, string password);

	// Login to server with a given username and password
	void login(string username, string password);

	// Open session with the given peer name
	bool openSession(string peerBName);

	// Print the current status of the client
	void printCurrentInfo();

	// Send UDP message to specific user or to all the users in a chat room
	bool sendMessage(string msg);

	// open session in the given room name
	bool createChatRoom(string name);

	// login to a room by it's room name
	bool loginToChatRoom(string roomName);

	// send to the server request of connected users
	void printConnectedUsersRequest();

	// print all of the connected users after received from the server
	void printConnectedUsers();

	// print the list of the users from the file
	void printListUsers();

	// send to the server request of room list
	void RoomsList();

	// send to the server request of all the users in a room (given by it's name)
	void listConnectedUsersInRoom(string roomName);

	// send to the server request of list of the users from the file
	void listUsers();

	// disconnect the open session / exit from a room
	bool exitRoomOrCloseSession();

	// delete the room by it's name (only the owner of the room can delete it)
	bool deleteChatRoom(string msg);

	// return true if the user is in a session or in a chat. false otherwise
	bool isInChat();

	// disconnect from the server and exit
	void exitAll();

	// update the user details after leaving a room
	void chatRoomLeaved();

	// update the rooms info
	void chatRoomUpdate();

	// print the list rooms
	void printRoomsList();

	bool isConnectedToServer() const;
};

#endif /* CLIENTMANAGER_H_ */
