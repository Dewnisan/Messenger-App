#include <iostream>

#include "Listener.h"
#include "LoginManager.h"
#include "MessengerServer.h"

using namespace std;

void printInstructions() {
	cout << "-------- SERVER MENU ---------" << endl;
	cout << "     Choose your command" << endl;
	cout << "lu              - list all users" << endl;
	cout << "lcu             - list all connected users" << endl;
	cout << "ls              - list all open sessions" << endl;
	cout << "lr              - list all rooms" << endl;
	cout << "lru <room name> - list all users in this room" << endl;
	cout << "x               - Shutdown server" << endl;
	cout << "p               - Print instructions" << endl;
	cout << "-------------------------------------------" << endl;
}

int main() {
	cout << "Welcome to TCP messenger Server" << endl;
	printInstructions();

	MessengerServer messengerServer("connctions.txt"); // listens to all of the sockets who have been connected (in the usersMap)
	LoginManager loginManager(&messengerServer); // handles the requests of all peer in the peer map (non-users)
	Listener listener(&loginManager); // listen to all of the new connections and adds the peers to the peer map.

	bool running = true;
	while (running) {
		string command;
		cin >> command;
		if (command == "lcu") { // List Connected Users
			messengerServer.getListConnectedUsers();
		} else if (command == "ls") { // List of all open sessions
			messengerServer.getListSessions();
		} else if (command == "lu") { // List of all Users from the file
			messengerServer.getListUsers();
		} else if (command == "lr") { // List of all the rooms
			messengerServer.getListRooms();
		} else if (command == "lru") {
			string chatRoom;
			cin >> chatRoom;

			messengerServer.getListChatUsers(chatRoom); // List of users in a specific room
		} else if (command == "p") {
			printInstructions();
		} else if (command == "x") {
			cout << "SHUTDOWN ALL..." << endl;
			running = false;
		} else {
			cout << "Wrong input - To print instructions type: p" << endl;
		}
	}

	cout << "messenger was closed" << endl;
	return 0;
}
