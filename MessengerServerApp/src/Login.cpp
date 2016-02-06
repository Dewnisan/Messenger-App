#include "Login.h"

/*
 * This class handles the login requests and the registration requests
 */

Login::Login(MessengerServer* serverManager)
{
	listener = NULL;
	_serverManager = serverManager;
	running = true;
	start();
}

void Login::run()
{
	TCPSocket* sockToTalk = NULL;
	int commandreceved = 0;
	string username;
	string password;
	while(running)
	{
		delete (listener);
		listener = new MultipleTCPSocketsListener();
		listener->addSockets(peers);

		sockToTalk = listener->listenToSocket(2);
		if (!sockToTalk)
			continue;

		commandreceved = sockToTalk->readCommand();
		switch  (commandreceved)
		{
		case LOGIN_REQUEST:
			username =  sockToTalk->readMsg();
			password =  sockToTalk->readMsg();
			if(login(username, password)) // user and password exist
			{
				if (_serverManager->isConnected(username)) // already logged in
				{
					sockToTalk->writeCommand(LOGIN_REQUEST_ALREADY_LOGGED);
				}
				else
				{
					_serverManager->addUser(sockToTalk, username);
					peers.erase(sockToTalk->destIpAndPort());
					sockToTalk->writeCommand(LOGIN_REQUEST_APPROVED);
				}
			}
			else // wrong details
			{
				sockToTalk->writeCommand(LOGIN_REQUEST_WRONG_DETAILS);
			}
			break;
		case SIGNUP_REQUEST:
			username =  sockToTalk->readMsg();
			password =  sockToTalk->readMsg();
			if (signUp(username,password))
				sockToTalk->writeCommand(SIGNUP_REQUEST_APPROVED);
			else
				sockToTalk->writeCommand(SIGNUP_REQUEST_DENIED);
			break;
		case EXIT:
			peers.erase(sockToTalk->destIpAndPort());
			cout << "peer had disconnected: "+ sockToTalk->destIpAndPort() << endl;
			break;
		}
	}
}

void Login::addPeer(TCPSocket* peer)
{
	peers[peer->destIpAndPort()] = peer;
}

Login::~Login()
{
	running = false;

	for (map<string,TCPSocket*>::iterator iter = peers.begin(); iter != peers.end(); iter++)
	{
		iter->second->cclose();
	}
	waitForThread();
}

// search in file for the given username and password.
bool Login::login(string userName,string password)
{
	string line;
	fstream loginFile;
	loginFile.open(_loginFile,ios::in|ios::out|ios::binary);

	if(loginFile.is_open()){
		while ( !loginFile.eof() )
		{
		  getline (loginFile,line);
		  if (line == userName+string("-") +password)
			  return true;
		}
		loginFile.close();
   }
   else
   {
	   cout <<"Error - could not open file!" << endl;
   }
		return false;
}

// write to file the new user with his details
bool Login::signUp(string userName,string password)
{
	string line;
	string userFromFile;
	fstream loginFile;
	loginFile.open(_loginFile,ios::in|ios::out|ios::binary);

	if (!loginFile.is_open()){
		   cout <<"Error - could not open file!" << endl;
		return false;
	}

	while ( !loginFile.eof() )
	{
		getline (loginFile,line);
		istringstream liness( line );
		getline( liness, userFromFile, ':' );

		if (userFromFile == userName)
		{
		    loginFile.close();
			return false;
		}
	}

	loginFile.close();

	ofstream loginFile1;
	loginFile1.open(_loginFile,ios::app);
	loginFile1<<userName+string("-")+password<<endl;
    loginFile1.close();
	return true;
}


Connection::Connection(Login* registration) {

	this->registration = registration;
	running = true;
	clientSocket = new TCPSocket(MSNGR_PORT);
	start();
	cout<<"server connection is up!"<<endl;

}

// waiting for connection from new peers and adds them to the peer list
void Connection::run()
{
	TCPSocket* temp;
	while (running)
	{
		temp = clientSocket->listenAndAccept();
		if(!temp)
			break;
		registration->addPeer(temp);
		cout<<"new peer has connected "<< temp->destIpAndPort()<< endl;
	}
	cout<<"the connection with server has stopped"<<endl;
}

Connection::~Connection() {
	running = false;
	clientSocket->cclose();
	waitForThread();
}

