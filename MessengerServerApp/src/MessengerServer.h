#ifndef MESSENGERSERVER_H_
#define MESSENGERSERVER_H_

#include <map>
#include <string>

#include "MessengerEntity.h"
#include "MThread.h"
#include "TCPSocket.h"
#include "User.h"

class MessengerServer: public MThread, public MessengerEntity {
	bool _running;
	std::string _pathToUsersFile; // TODO: add locking to file

	std::map<std::string, User*> _users; // pool of logged in users
	std::map<std::string, ChatRoom*> _chatRooms;


	void printToScreen(string msgToScreen);

	// Returns the number of registered users
	int numOfUsersFromFile();

	// Prints/sends the names of all the users that are registered
	void readFromFile(User *clientName);

	// Prints/sends the names of all the chat rooms that exists
	void readFromChatRoom(User *clientName);

	// Create a session between two users
	void createSession(User* initiatingUser, User* targetUser);

	// Handles the users requests
	void run();

public:

	MessengerServer(const std::string& pathToUsersFile);
	virtual ~MessengerServer();

	// Prints List of users from login file
	void listUsers();

	// Prints List of the connected users
	void listConnectedUsers();

	// Prints list of all open sessions
	void listSessions();

	// Prints list of all chat rooms
	void listChatRooms();

	// Prints list of users in a specific chat room
	void listChatRoomUsers(string chatRoomName);

	// Adds user to users pool
	bool addUser(TCPSocket* userSocket, std::string userName);

	// Send list of the connected users to the asking client
	void sendListConnectedUsers(User *client);

	bool isConnected(string userName);

	// Send list of chat rooms to the asking client
	void sendListChatRooms(User *client);

	// Send list of users in a specific chat room to the asking client
	void sendListChatRoomUsers(User *client);

	// Send List of users from login file to asking client
	void sendListUsers(User *clientName);

	// User exit from the server
	void exitServer(User* client);

	// Create new chat room
	void createChatRoom(User* creator);

	// Delete chat room
	void deleteChatRoom(User* creator);

	// Enter to chat room
	void enterChatRoom(User* creator);
};

#endif /* MESSENGERSERVER_H_ */
