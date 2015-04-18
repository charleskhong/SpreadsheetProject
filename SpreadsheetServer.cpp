#include "SpreadsheetServer.h"


using namespace std;

void dostuff(int);
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
  //server.userList.insert("sysadmin");


  // Load all spreadsheets and users


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

  registered_users.insert("sysadmin");

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

/**
 *
 *
 */
void SpreadsheetServer::openSpreadsheet(int client_socket, std::string filename)
{
  const char* file = filename.c_str();
  // Check to see if the filename is valid
  bool exists = false;
  int numcells = 8;
  Spreadsheet s;

  spreadsheets_lock.lock();
  int total_sheets = open_spreadsheets.size();
  for (int i = 0; i < open_spreadsheets.size(); i++)
    {
      // If the spreadsheet is in the list
      if (strcmp(open_spreadsheets.at(i).filename, file) == 0)
	{
	  exists = true;
	  	  s = open_spreadsheets.at(i);
	  // Spreadsheet make a function to return number of cells
	  	  numcells = s.cells.size();
	  break;
	}
    }
  spreadsheets_lock.unlock();

  // If the spreadsheet has not been opened before
  if (!exists)
    {
      
      // Load the spreadsheet if it exists on disk, otherwise this will return a new spreadsheet
      s = Spreadsheet(file); 
      numcells = s.cells.size();
      // Make Spreadsheet use a const char* instead

      // Spreadsheet is now an active spreadsheet
      spreadsheets_lock.lock();
      open_spreadsheets.push_back(s);
      spreadsheets_lock.unlock();
      // Indicate the connection between socket and the spreadsheet
      connections_lock.lock();
      sprd_connections.insert(std::pair<int, Spreadsheet>(client_socket, s));
      connections_lock.unlock();
      
    }

  sendConnected(client_socket, numcells);

   // Send all non-empty cells and their contents to the client
  for (map<string, string>::iterator it = s.cells.begin(); it != s.cells.end(); it++)
    {
      string cell = it->first;
      string content = it->second;

      sendCell(client_socket, cell, content);
      } 

  // Send the cells
  /**sendCell(client_socket, "A1", "This");
  sendCell(client_socket, "A2", "is");
  sendCell(client_socket, "A3", "a");
  sendCell(client_socket, "A4", "preloaded");
  sendCell(client_socket, "A5", "spreadsheet");
  sendCell(client_socket, "A6", "=3*3");**/
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

  // Read all the commands
  // If a newline is not at the end continue reading
  // 
  n = read(client_socket, buffer, 1024);
  if (n > 0)
    {
      printf(buffer);
      for (int i = 0; i < n; i++)
	{
	  char c = buffer[i];
	  if (c == '\n')	    
	      break;
	  else
	    line.append(1, c);
	}
    }
  /*  while (n > 0)
    {
      printf(buffer);
      for (int i = 0; i < n; i++)
	{
	  char c = buffer[i];
	  if (c == '\n')
	    break;
	  else
	    line.append(1,c);
	}
      n = read(client_socket, buffer, 1024);
      }*/
  cout << line << endl;
  std::stringstream ss(line);
  std::string token;
  while (ss >> token)
    tokens.push_back(token);
  
  // Delimits by space rather than whitespace
  /*while (getline(ss, token, ' '))
    tokens.push_back(token);*/
  std::string command (tokens.at(0));
  if (command.compare("connect") == 0)
    {
      cout << "connect received" << endl;
      connectReceived(client_socket, tokens);
      // connect received
    }
  else if (command.compare("register") == 0)
    {
      cout << "register received" << endl;
      registerReceived(client_socket, tokens);
      // register received
    }
  else if (command.compare("cell") == 0)
    {
      cout << "cell received" << endl;
      cellReceived(client_socket, tokens);
      // cell command
    }
  else if (command.compare("undo") == 0)
    {
      cout << "undo received" << endl;
      undoReceived(client_socket, tokens);
      // undo command
    }
  else
    {
      sendError(client_socket, 2, "Invalid command");
      // unrecognized command
    }

  messageReceived(client_socket);
}

void SpreadsheetServer::connectReceived(int client_socket, std::vector<std::string> tokens)
{
  std::string user, filename;

  if (tokens.size() != 3)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }
  user = tokens.at(1);
  filename = tokens.at(2);

  bool registered = false;
  users_lock.lock();
  registered = registered_users.find(user) != registered_users.end();
  users_lock.unlock();

  // Not registered
  if (!registered)
      sendError(client_socket, 4, "Username is not registered or is taken");
  else
    openSpreadsheet(client_socket, filename);

}

void SpreadsheetServer::registerReceived(int client_socket, std::vector<std::string> tokens)
{
  std::string username;

  if (tokens.size() == 1)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }
  else if (tokens.size() > 2)
    {
      sendError(client_socket, 4, "Invalid username");
      return;
    }

  username = tokens.at(1);

  // Register the user
  // ret.second - true if newly inserted. false if already existed
  users_lock.lock();
  std::pair<std::set<std::string>::iterator, bool> ret = registered_users.insert(username);
  users_lock.unlock();

  // User was already registered
  if (ret.second == false)
    sendError(client_socket, 4, "Username already registered");

}

void SpreadsheetServer::cellReceived(int client_socket, std::vector<std::string> tokens)
{
  std::string cell, contents;

  if (tokens.size() != 3)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }
  cell = tokens.at(1);
  contents = tokens.at(2);
  
  /**connections_lock.lock();
  Spreadsheet s = sprd_connections.find(client_socket)->second;
  connections_lock.unlock();
  // sprd_connections[client_socket] gives second
  s.setCell(cell, contents);**/
  

  // For now echo the changes back to the client
  sendCell(client_socket, cell, contents);

}

void SpreadsheetServer::undoReceived(int client_socket, std::vector<std::string> tokens)
{
  if (tokens.size() != 1)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }

  /*
  connections_lock.lock();
  Spreadsheet s = sprd_connections.find(client_socket)->second;
  connections_lock.unlock();
  s.undo();
  */
}

void SpreadsheetServer::sendConnected(int client_socket, int numcells)
{
  int length, n;
  char message[256];
  length = sprintf(message, "connected %d\n", numcells);
  if (length < 0)
    error("ERROR creating connected message");
  n = write(client_socket, message, length);
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

  if (n < 0)
    error("ERROR writing to socket");

}
