#ifndef CHATROOM_H_
#define CHATROOM_H_

#include <string>
#include <vector>

#include "User.h"

class User;

class ChatRoom {
	std::string _name;
	std::vector<User*> _users;
	User* _owner;

	void updateUsers();

public:
	ChatRoom(User* owner, std::string name);
	virtual ~ChatRoom();

	User* getOwner();

	bool  addUser(User* user);
	bool  removeUser(User *user);

	int  printUsers();
	void  sendUserList(User *client);
};

#endif /* CHATROOM_H_ */
