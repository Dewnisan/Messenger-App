#include <iostream>

#include "MessengerClient.h"

using namespace std;

void printInstructions() {
	cout << "------------------------ CLIENT MENU ----------------------" << endl;
	cout << "                  Choose your command" << endl;
	cout << "c <server ip>                   -  Connect to server" << endl;
	cout << "register <username> <password>  -  Register a new user" << endl;
	cout << "login <username> <password>     -  Login to server" << endl;
	cout << "lu                              -  Print the user list" << endl;
	cout << "lcu                             -  Print the connected user list" << endl;
	cout << "cr <room name>                  -  Create a new chat room" << endl;
	cout << "or <room name>                  -  Enter a chat room" << endl;
	cout << "dr <room name>                  -  Delete a chat room" << endl;
	cout << "lr                              -  Print all chat rooms" << endl;
	cout << "lru <room name>                 -  print all users in a given chat room" << endl;
	cout << "o <username>                    -  Open a session with a user" << endl;
	cout << "s <message>                     -  Send a message After the session is opened" << endl;
	cout << "cs                              -  Close opened session or exit from a room" << endl;
	cout << "l                               -  Print the current status of the client" << endl;
	cout << "d                               -  Disconnect from server" << endl;
	cout << "x                               -  Close the application" << endl;
	cout << "p                               -  Print instructions" << endl;
	cout << "-----------------------------------------------------------" << endl;
}

int main() {
	cout << " Welcome to the client messenger" << endl;

	printInstructions();
	MessengerClient clientManager;

	bool running = true;
	while (running) {
		string command, parameter1, parameter2, answer;
		cin >> command;
		if (command == "c") {
			cin >> parameter1; // Server IP
			if (clientManager.connectToServer(parameter1, MSNGR_PORT)) {
				cout << "Connected to: " << parameter1 << endl;
			} else {
				cout << "Connection failed - already connected" << endl;
			}
		} else if (command == "login") {
			cin >> parameter1; // Username
			cin >> parameter2; // Password
			clientManager.login(parameter1, parameter2);
		} else if (command == "register") {
			cin >> parameter1; // Username
			cin >> parameter2; // Password
			clientManager.signup(parameter1, parameter2);
		} else if (command == "lu") {
			clientManager.listUsers();
		} else if (command == "lcu") {
			clientManager.listConnectedUsers();
		} else if (command == "cr") {
			cin >> parameter1; // Room name
			if (!clientManager.createChatRoom(parameter1)) {
				cout << "Cannot create room in current status, check if you are logged in" << endl;
			}
		} else if (command == "or") {
			cin >> parameter1; // Room name
			if (!clientManager.enterChatRoom(parameter1)) {
				cout << "Cannot enter to the room, check if you are already logged in" << endl;
			}
		} else if (command == "dr") {
			cin >> parameter1; // Room name
			if (!clientManager.deleteChatRoom(parameter1)) {
				cout << "Cannot delete specified room" << endl;
			}
		} else if (command == "lr") {
			clientManager.listRooms();
		} else if (command == "lru") {
			cin >> parameter1; // Room name
			clientManager.listRoomUsers(parameter1);
		} else if (command == "o") {
			cin >> parameter1; // Username to open session with
			clientManager.openSession(parameter1);
		} else if (command == "s") {
			getline(std::cin, parameter1); // Message to send with UDP
			clientManager.sendMessage(parameter1);
		} else if (command == "cs") {
			if (!clientManager.closeSessionOrExitRoom()) {
				cout << "There is not session or room to exit from" << endl;
			}
		} else if (command == "l") {
			clientManager.printCurrentInfo();
		} else if (command == "d") {
			if (clientManager.connectedToServer()) {
				clientManager.disconnectFromServer();
				cout << "Disconnection from the server completed" << endl;
			} else {
				cout << "Already disconnected from the server" << endl;
			}
		} else if (command == "x") {
			if (clientManager.connectedToServer()) {
				clientManager.disconnectFromServer();
			}

			cout << "Shutting down..." << endl;
			running = false;
			clientManager.waitForThread();
		} else if (command == "p") {
			printInstructions();
		} else {
			cout << "Wrong input - to print instructions type: p" << endl;
		}
	}
}
