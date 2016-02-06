#ifndef CHATROOM_H_
#define CHATROOM_H_

#include <string>
#include <vector>

#include "User.h"

class User;

class ChatRoom {
	std::string _chatRoomName;
	std::vector<User*> charRoomUsers;
	User* _roomOwner;
	void updateusers();

public:
	ChatRoom(User* owner, std::string chatroom);
	virtual ~ChatRoom();

	bool  logOffUser(User *usertologof);
	bool  addUser(User* userToAdd);
	User* getOwner();
	int   sendUserList(User *sendto);
	int  printUsers();

};

#endif /* CHATROOM_H_ */
