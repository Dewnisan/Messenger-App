#ifndef MESSENGERSERVER_H_
#define MESSENGERSERVER_H_

#include <map>
#include <string>

#include "MThread.h"
#include "TCPMessengerProtocol.h"
#include "TCPSocket.h"
#include "User.h"

class User;

class MessengerServer: public MThread {
	bool _running;
	std::string _pathToUsersFile;

	std::map<std::string, User*> _users; // map of logged in users
	std::map<std::string, ChatRoom*> _chatRooms;

	// Create a session between two users
	void createSession(User* initiatingUser, User* targetUser);

public:

	MessengerServer(const std::string& pathToUsersFile);
	virtual ~MessengerServer();

	// Handles the users requests
	void run();

	// add user to users map
	bool addUser(TCPSocket* userSocket, string LoginUserName);

	// user exit from the server
	void exitServer(User* clientName);

	// print/send all the users that are registered (in the file)
	void readFromFile(User *clientName);

	// print/send all the rooms
	void readfromChatRoom(User *clientName);

	//void readfromUsers(User *clientName);

	void printToSreen(string msgToScreen);

	// returns the number of registered users
	int numOfUsersFromFile();

	//int  numOfUserFromList();

	// Print List of the connected users
	int listConnectedUsers();

	// Send list of the connected users to the asking client
	int getListConnectedUsers(User *client);

	bool isConnected(string username);

	// get list of all open sessions
	void listSessions();

	// Print list of all chat rooms
	void listRooms();

	// Send  list of chat rooms to the asking client
	void getListRooms(User *clientName);

	// Print list of users in a specific room
	int listChatUsers(string ChatRoomName);

	// Send list of users in a specific room to the asking client
	int getListChatUsers(User *clientName);

	// Print List of users from login file
	void listUsers();

	// Send List of users from login file to asking client
	void getListUsers(User *clientName);

	// Create new chat room
	void createChatRoom(User* creator);

	// delete chat room
	void deleteChatRoom(User* creator);

	// login to chat room
	void loginChatRoom(User* creator);

};

#endif /* MESSENGERSERVER_H_ */
