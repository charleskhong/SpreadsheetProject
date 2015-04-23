#include "SpreadsheetServer.h"


using namespace std;

void error(const char *msg)
{
  perror(msg);
  exit(1);
}

int main(int argc, char *argv[])
{
  if (argc < 2)
    {
      cout << "Please provide a port number for the server" << endl;
      return 0;
    }

  SpreadsheetServer server (atoi(argv[1]));
  server.start();

  return 0;
}


SpreadsheetServer::SpreadsheetServer()
{

}

SpreadsheetServer::SpreadsheetServer(int port)
{
  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0)
    error("ERROR opening socket");

  bzero((char *) &server_addr, sizeof(server_addr));

  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (::bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    error("ERROR on binding");
  if(!load_users())
    {
      registered_users.insert("sysadmin");
      save_users();
    }
  else
    {
      cout<<"Loaded users successfully"<<endl;
    }

  cout << "Server initialized" << endl;
}

SpreadsheetServer::~SpreadsheetServer()
{
}

void SpreadsheetServer::start()
{
  socklen_t client_length;
  struct sockaddr_in client_addr;
  int client_socket, pid;

  listen(server_socket, 5);
  client_length = sizeof(client_addr);

  cout << "Server listening" << endl;


  while (1)
    {
      client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_length);
      if (client_socket < 0)
	error("ERROR on accept");

      // Params: function, this object, function parameters
      std::thread (&SpreadsheetServer::messageReceived, this, client_socket).detach();
    }
  close(server_socket);
}


bool SpreadsheetServer::save_users()
{
  //cells["R5"] = "768";

  ofstream userfile ("usernames.usrs");
  if(userfile.is_open()){
    for (std::set<string>::iterator it=registered_users.begin(); it!=registered_users.end(); ++it)
      {
	userfile << *it+ "\n";
      }
    userfile.close();
    return true;
  }
  else {
    // File didn't open
    cout << "Unable to open user file" << endl;
    return false;
  }
  
}

bool SpreadsheetServer::load_users()
{
  string line;
  ifstream userfile ("usernames.usrs");
  if (userfile.is_open())
    {
      while (getline (userfile,line) )
	{
	  std::string username;
	  stringstream ss(line);
	  
	  ss>>username;
	  registered_users.insert(username);
	  
	}
      userfile.close();
      return true;
    }
  else
    {
      cout<<"Username file does not exist"<<endl;
      return false;
    }
}

/**
 *
 *
 */
void SpreadsheetServer::openSpreadsheet(int client_socket, std::string filename)
{
  const char* file = filename.c_str();
  // Check to see if the filename is valid
  bool exists = false;
  int numcells;

  spreadsheets_lock.lock();

  // Active spreadsheet
  for (int i = 0; i < open_spreadsheets.size(); i++)
    {
      // If the spreadsheet is in the list
      if (strcmp(open_spreadsheets.at(i).filename, file) == 0)
	{
	  exists = true;

	  // Add this socket to the spreadsheet
	  open_spreadsheets.at(i).sockets.push_back(client_socket);

	  // Send the connect command with number of cells in spreadsheet
     	  numcells = open_spreadsheets.at(i).cells.size();
	  sendConnected(client_socket, numcells);

	  // Send all non-empty cells and their contents to the client
	  for (map<string, string>::iterator it = open_spreadsheets.at(i).cells.begin(); it != open_spreadsheets.at(i).cells.end(); it++)
	    {
	      string cell = it->first;
	      string content = it->second;
	      
	      sendCell(client_socket, cell, content);
	    }
	  break;
	}
    }

  // If the spreadsheet has not been opened yet
  if (!exists)
    {      
      // Add to active spreadsheets
      open_spreadsheets.push_back(Spreadsheet(file));
      int back = open_spreadsheets.size()-1;

      // Add this socket to the spreadsheet
      open_spreadsheets.at(back).sockets.push_back(client_socket);

      // Send the connect command with number of cells in spreadsheet
      numcells = open_spreadsheets.at(back).cells.size();
      sendConnected(client_socket, numcells);

      // Send all non-empty cells and their contents to the client
      for (map<string, string>::iterator it = open_spreadsheets.at(back).cells.begin(); it != open_spreadsheets.at(back).cells.end(); it++)
	{
	  string cell = it->first;
	  string content = it->second;
	  
	  sendCell(client_socket, cell, content);
	}
    }
  spreadsheets_lock.unlock();

  // Establish the connection between socket and the spreadsheet
  connections_lock.lock();
  sprd_connections.insert(std::pair<int, const char*>(client_socket, file));
  connections_lock.unlock();

}

/**
 *
 *
 *
 */
void SpreadsheetServer::messageReceived(int client_socket)
{
  std::string line, next;
  char buffer[1024];
  int n = 0;
  std::vector<std::string> tokens;

  // Read one byte at a time from client
  n = read(client_socket, buffer, 1);
  cout << "Client: ";
  while (n > 0)
    {
      cout << buffer;
      char c = buffer[0];
      if (c == '\n')
	break;
      else
	line.append(1, c);
      n = read(client_socket, buffer, 1);
    }
  
  // If an error occured or client disconnected
  if (n <= 0)
    {
       cout << "client disconnected" << endl;

       /** what if they have not connected to a spreadsheet  **/
       // Remove connection of clent and their spreadsheet
       connections_lock.lock();
       const char * filename = sprd_connections.find(client_socket)->second;
       sprd_connections.erase(client_socket);
       connections_lock.unlock();

       // Remove the socket from spreadsheet they were working on
       spreadsheets_lock.lock();
       for (int i = 0; i < open_spreadsheets.size(); i++)
	 {
	   // Compare by filename
	   if (strcmp(open_spreadsheets.at(i).filename, filename) == 0)
	     {
	       // Erase the socket
	       open_spreadsheets.at(i).sockets.erase(find(open_spreadsheets.at(i).sockets.begin(), open_spreadsheets.at(i).sockets.end(), client_socket));
	       // If there are no more sockets associated with this spreadsheet it is no longer open/active
	       if(open_spreadsheets.at(i).sockets.size() == 0)
		 {
		   open_spreadsheets.erase(open_spreadsheets.begin()+i);
		 }
	     }
	 }
       spreadsheets_lock.unlock();

       return;
    }

  // Split by whitespace
  std::stringstream ss(line);
  std::string token;
  while (ss >> token)
    tokens.push_back(token);
  
  // Delimits by space rather than whitespace
  /*while (getline(ss, token, ' '))
    tokens.push_back(token);*/

  std::string command;

  if (tokens.size() > 0)
    command = (tokens.at(0));
  else
    command = "";

  if (command.compare("connect") == 0)
    {
      cout << "connect received" << endl;
      connectReceived(client_socket, tokens);
    }
  else if (command.compare("register") == 0)
    {
      cout << "register received" << endl;
      registerReceived(client_socket, tokens);
    }
  else if (command.compare("cell") == 0)
    {
      cout << "cell received" << endl;
      cellReceived(client_socket, tokens);
    }
  else if (command.compare("undo") == 0)
    {
      cout << "undo received" << endl;
      undoReceived(client_socket, tokens);
    }
  else
    {
      sendError(client_socket, 2, "Invalid command");
    }

  messageReceived(client_socket);
}

void SpreadsheetServer::connectReceived(int client_socket, std::vector<std::string> tokens)
{
  std::string user, filename;

  // Check to see if client has connected
  spreadsheets_lock.lock();
  int con = sprd_connections.count(client_socket);
  spreadsheets_lock.unlock();

  // Already connected
  if (con == 1)
    {
      sendError(client_socket, 3, "Already connected to a spreadsheet");
      return;
    }

  // Too many or too few arguments
  if (tokens.size() != 3)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }

  user = tokens.at(1);
  filename = tokens.at(2);

  // Check to see if user has been registered
  bool registered = false;
  users_lock.lock();
  registered = registered_users.find(user) != registered_users.end();
  users_lock.unlock();

  // Not registered
  if (!registered)
      sendError(client_socket, 4, "Username is not registered or is taken");

  // Registered
  else
    openSpreadsheet(client_socket, filename);

}

void SpreadsheetServer::registerReceived(int client_socket, std::vector<std::string> tokens)
{
  std::string username;

  // Check to see if client has connected
  spreadsheets_lock.lock();
  int con = sprd_connections.count(client_socket);
  spreadsheets_lock.unlock();

  // Not connected
  if (con != 1)
    {
      sendError(client_socket, 3, "Cannot perform command before connecting to a spreadsheet");
      return;
    }

  // Only sent the register command
  if (tokens.size() == 1)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }

  // Send more than register command or name had spaces
  else if (tokens.size() > 2)
    {
      sendError(client_socket, 4, "Invalid username");
      return;
    }

  username = tokens.at(1);

  // Register the user and save to file
  users_lock.lock();
  std::pair<std::set<std::string>::iterator, bool> ret = registered_users.insert(username);
  save_users();
  users_lock.unlock();

  // User was already registered
  if (ret.second == false)
    sendError(client_socket, 4, "Username already registered");

}

void SpreadsheetServer::cellReceived(int client_socket, std::vector<std::string> tokens)
{
  std::string cell, contents;

  // Check to see if client has connected
  connections_lock.lock();
  int con = sprd_connections.count(client_socket);
  connections_lock.unlock();

  // not connected
  if (con != 1)
    {
      sendError(client_socket, 3, "Cannot perform command before connecting to a spreadsheet");
      return;
    }

  // Only sent 'cell\n'
  if (tokens.size() == 1)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }

  // 'cell A1 ""\n'
  // Setting contents of a cell to ""
  else if (tokens.size() == 2)
    contents = "";

  // Setting contents of a cell regularly
  else
    {
      // Grab all the contents from the command
      for (int i = 2; i < tokens.size(); i++)
	contents += tokens.at(i) + " ";

      // Remove excess " " at the end
      contents = contents.substr(0, contents.size()-1);
    }

  cell = tokens.at(1);

  // Check to see that the cell name is syntactically correct
  int chars = cell.size();
  char c = cell[0];

  // Cell name should have either 2 or 3 characters
  if (cell.size() != 2 && cell.size() != 3)
    {
      sendError(client_socket, 2 ,"Invalid cell name");
      return;
    }

  // Does not start with a letter
  if (!(c >= 65 && c <= 90) && !(c >= 97 && c <= 122))
    {
      sendError(client_socket, 2 ,"Invalid cell name");
      return;
    }


  // Check remaining characters for whether or not they are numbers
  for (int i = 1; i < chars; i++)
    if (!(cell[i] >= 48 && cell[i] <= 57))
      {
	sendError(client_socket, 2, "Invalid cell name");
	return;
      }
  
  // Find spreadsheet associated with socket
  connections_lock.lock();
  const char * filename = sprd_connections.find(client_socket)->second;
  connections_lock.unlock();

  // Find the spreadsheet
  spreadsheets_lock.lock();
  for (int i = 0; i < open_spreadsheets.size(); i++)
    {
      if (strcmp(filename, open_spreadsheets.at(i).filename) == 0)
	{
	  // Set contents of cell
	  bool c = open_spreadsheets.at(i).setCell(cell, contents);

	  // Successfully changed contents
	  if (c)
	    {	     
	      // Send change to every socket associated with client
	      vector<int> sockets = open_spreadsheets.at(i).sockets;

	      for(int i = 0; i < sockets.size(); i++)
		{
		  cout << sockets.at(i) << endl;
		  sendCell(sockets.at(i), cell, contents);
		}    
	    }
	  else
	    sendError(client_socket, 1, "Circular dependency occurs with this change");
	  break;
	}
    }
  spreadsheets_lock.unlock();

}

void SpreadsheetServer::undoReceived(int client_socket, std::vector<std::string> tokens)
{
  // Check to see client has connected
  spreadsheets_lock.lock();
  int con = sprd_connections.count(client_socket);
  spreadsheets_lock.unlock();

  // Not connected
  if (con != 1)
    {
      sendError(client_socket, 3, "Cannot perform command before connecting to a spreadsheet");
      return;
    }

  // More than just "undo\n"
  if (tokens.size() != 1)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }

  connections_lock.lock();
  const char* filename = sprd_connections.find(client_socket)->second;
  connections_lock.unlock();

  spreadsheets_lock.lock();
  for (int i = 0; i < open_spreadsheets.size(); i++)
    {
      if (strcmp(filename, open_spreadsheets.at(i).filename) == 0)
	{
	  std::pair<std::string, std::string> change;
	  change = open_spreadsheets.at(i).undo();

	  string name, contents;
	  name = change.first;
	  contents = change.second;

	  if (name.compare("ERROR") != 0)
	    {
	      vector<int> sockets = open_spreadsheets.at(i).sockets;
	      for (int i = 0; i < sockets.size(); i++)
		sendCell(sockets.at(i), name, contents);
	    }
	  else
	    sendError(client_socket, 3, "Cannot undo before a change has been made");

	  return;
	}
    }
  
}

void SpreadsheetServer::sendConnected(int client_socket, int numcells)
{
  int length, n;
  char message[256];
  length = sprintf(message, "connected %d\n", numcells);
  if (length < 0)
    error("ERROR creating connected message");
  n = write(client_socket, message, length);
  cout << "Server: " << message;
  if (n < 0)
    error("ERROR writing to socket");
}

void SpreadsheetServer::sendCell(int client_socket, std::string cell_name, std::string contents)
{
  int length, n;
  const char * cell = cell_name.c_str();
  const char * content = contents.c_str();
  char message[256];
  length = sprintf(message, "cell %s %s\n", cell, content);
  if (length < 0)
    error("ERROR creating send cell message");

  n = write(client_socket, message, length);
  cout << "Server: " << message;

  if (n < 0)
    error("ERROR writing to socket");
}

void SpreadsheetServer::sendError(int client_socket, int error_num, std::string info)
{
  int n, length;
  const char * err = info.c_str();
  char message[256];
  length = sprintf(message, "error %d %s\n", error_num, err);
  if (length < 0)
    error("ERROR creating client error message");

  n = write(client_socket, message, length);
  cout << "Server: " << message;

  if (n < 0)
    error("ERROR writing to socket");

}
