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
	cout << "Welcome to messenger server" << endl;
	printInstructions();

	MessengerServer messengerServer("connctions.txt"); // Listens to all of the users which have been connected
	LoginManager loginManager(&messengerServer); // Handles the requests of all peer in the peer pool (non-users)
	Listener listener(&loginManager); // Listen to all of the new connections and adds the peers to the peer pool

	bool running = true;
	while (running) {
		string command;
		cin >> command;

		if (command == "lu") {
			messengerServer.listUsers();
		} else if (command == "lcu") {
			messengerServer.listConnectedUsers();
		} else if (command == "ls") {
			messengerServer.listSessions();
		} else if (command == "lr") {
			messengerServer.listChatRooms();
		} else if (command == "lru") {
			string chatRoomName;
			cin >> chatRoomName;
			messengerServer.listChatRoomUsers(chatRoomName);
		} else if (command == "x") {
			cout << "Server is shutting down..." << endl;
			running = false;
		} else if (command == "p") {
			printInstructions();
		} else {
			cout << "Wrong input - to print instructions type: p" << endl;
		}
	}

	cout << "Messenger was closed" << endl;
	return 0;
}
