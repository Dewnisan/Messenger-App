#include "ChatRoom.h"
#include "TCPMessengerProtocolExtentions.h"

ChatRoom::ChatRoom(User* owner, string chatRoom) {
	_roomOwner = owner;
	_chatRoomName = chatRoom;
}

ChatRoom::~ChatRoom() {

	for(vector<User*>::iterator iter = charRoomUsers.begin(); iter != charRoomUsers.end(); iter++)
	{
		(*iter)->disconnectFromChatRom(true);
	}
}

void ChatRoom::updateusers()
{
	for(vector<User*>::iterator iter = charRoomUsers.begin(); iter != charRoomUsers.end(); iter++)
	{
		(*iter)->writeCommand(CHAT_ROOM_UPDATED);
		(*iter)->writeMsg(_chatRoomName);
		(*iter)->writeCommand(charRoomUsers.size());

		for(vector<User*>::iterator iter2 = charRoomUsers.begin(); iter2 != charRoomUsers.end(); iter2++)
		{
			(*iter)->writeMsg((*iter2)->getusername());
			(*iter)->writeMsg((*iter2)->getIP());
			(*iter)->writeCommand((*iter2)->getport());
		}
	}
}

int ChatRoom::printUsers()
{
	int count = 0;
	for(vector<User*>::iterator iter = charRoomUsers.begin(); iter != charRoomUsers.end(); iter++)
	{
		cout<<(*iter)->getusername()<<endl;
		count++;
	}
	return count;
}

// send list of users in the same
int ChatRoom::sendUserList(User *sendto)
{
	int numOfUsers = 0;


	sendto->writeCommand(charRoomUsers.size());
	for(vector<User*>::iterator iter = charRoomUsers.begin(); iter != charRoomUsers.end(); iter++)
	{
		sendto->writeMsg((*iter)->getusername());
		numOfUsers++;
	}

	return numOfUsers;
}

// add user to room
bool ChatRoom::addUser(User* userToAdd)
{
	bool exist = false;

	// check if exist
	for(vector<User*>::iterator iter = charRoomUsers.begin(); iter != charRoomUsers.end(); iter++)
	{
		if ((*iter) == userToAdd)
		{
			exist = true;
		}

	}


	if(exist)
	{
		cout << "User already exist" <<endl;
		return false;
	}

	charRoomUsers.push_back(userToAdd);
	updateusers();

	return true;
}

// return room owner
User* ChatRoom::getOwner()
{
	return _roomOwner;
}

// log off a user from the server
bool ChatRoom::logOffUser(User *usertologof)
{
	bool exist = false;

	// check if exist
	for(vector<User*>::iterator iter = charRoomUsers.begin(); iter != charRoomUsers.end(); iter++)
	{
		if ((*iter) == usertologof)
		{
			exist = true;
		}

	}

	if(!exist){
		return false;
	}

	for(vector<User*>::iterator iter = charRoomUsers.begin(); iter != charRoomUsers.end(); iter++)
		{
		if ((*iter) == usertologof)
		{
			charRoomUsers.erase(iter);
			break;
		}
	}
	updateusers();
	return true;
}

