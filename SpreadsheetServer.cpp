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

  if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
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

  //  while (1)
  //{
      client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_length);
      if (client_socket < 0)
	error("ERROR on accept");

      connectionReceived(client_socket);

      //messageReceived(client_socket);


  close(server_socket);
}

void SpreadsheetServer::connectionReceived(int client_socket)
{
  int n;
  char buffer[256];
  char * token;
  std::vector<char*> tokens;
  char* command;
  char* user;
  char* file;

  bzero(buffer, 256);
  n = read(client_socket, buffer, 256);

  if (n < 0)
    error("ERROR reading from socket");

  printf("Here is the command: %s\n", buffer);

  // Split string by delimiter " "
  token = strtok(buffer, " ");

  // End of string will always return NULL
  while (token != NULL)    
    {
      tokens.push_back(token);
      token = strtok(NULL, " ");
    }
  command = tokens.at(0);  

  if (strcmp(command, "connect") == 0)
    {
      // Incorrect number of tokens
      if (tokens.size() != 3)
	{
	  sendError(client_socket, 2, "Incorrect number of tokens");
	  connectionReceived(client_socket);
	}
    
      user = tokens.at(1);
      file = tokens.at(2);

      // Username is not registered
      if (registered_users.find(user) == registered_users.end())
	{
	  sendError(client_socket, 4, "Username is not registered or is taken");
	  connectionReceived(client_socket);
	    
	}

      cout << "Username was not taken" << endl;
      openSpreadsheet(client_socket, file);

    }

  // First token is not connect
  else if (strcmp(tokens.at(0), "connect") != 0)
    {
      sendError(client_socket, 2, "Expected connect command");
      connectionReceived(client_socket);
    }

  // ? Close the connection at this point ?
  if (n < 0)
    error("ERROR writing to socket");
   
}

void SpreadsheetServer::commandReceived(int client_socket)
{
  int n;
  char buffer[256];
  char msg[256];
  char* token;
  char* command;
  std::vector<char*> tokens;

  bzero(buffer, 256);
  n = read(client_socket, buffer, 256);
  printf("Here is the command: %s\n", buffer);

  // Split string by delimiter " "
  token = strtok(buffer, " ");

  // End of string will always return NULL
  while (token != NULL)    
    {
      tokens.push_back(token);
      token = strtok(NULL, " ");
    }

  command = tokens.at(0);

  if (strcmp(command, "register") == 0)
    {
      if (tokens.size() != 2)
	sendError(client_socket, 2, "Incorrect number of tokens");

      else
	{
	  char* name = tokens.at(1);

	  // ? Do we need to send an error if the username is already registered ?
	  registered_users.insert(name);
	  
	  int l = sprintf(msg, "%s %s\n", command, name);

	  // Form a indicator message, for debugging
	  n = write(client_socket, msg, l);
	}
    }
  else if (strcmp(command, "cell") == 0)
    {
      if (tokens.size() != 3)
	sendError(client_socket, 2, "Incorrect number of tokens");

      else
	{
	  char* cell = tokens.at(1);
	  char * contents = tokens.at(2);
      
	  int l = sprintf(msg, "%s %s %s\n", command, cell, contents);

	  // Form a indicator message, for debugging
	  n = write(client_socket, msg, l);
	}
    }
  else if (tokens.size() == 1)//strcmp(command, "undo") == 0)
    {
      int len = strlen(tokens.at(0));
      if (len != 6)
      {
	  sendError(client_socket, 2, "Incorrect number of tokens");
      }
      else
	{
	  char * undo = "undo";
	  bool un = true;
	  for (int i = 0; i < 4; i++)
	    {
	      if (command[i] != undo[i])
		{
		  un = false;
		  break;
		}
	    }
	  if (un)
	    {
	      // Send the command to the spreadsheet to make changes
	      int l = sprintf(msg, "%s\n", command);

	      // Form a indicator message, for debugging
	      n = write(client_socket, msg, l);
	    }
	  else
	    sendError(client_socket, 2, "Command not recognized");
	}
    }
  else
    {
      sendError(client_socket, 2, "Command not recognized");
    }

  if (n < 0)
    error("ERROR reading from socket");

  // Continue listening for more commands
  commandReceived(client_socket);

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
  /// Spreadsheet s;

  for (int i = 0; i < open_spreadsheets.size(); i++)
    {
      // If the spreadsheet is in the list
      if (strcmp(open_spreadsheets.at(i).filename, file) == 0)
	{
	  exists = true;
	  ///	  s = open_spreadsheets.at(i);
	  // Spreadsheet make a function to return number of cells
	  ///	  numcells = s.cells.size();
	  break;
	}
    }

  // If the spreadsheet has not been opened before
  if (!exists)
    {
      /***
      // Load the spreadsheet if it exists on disk, otherwise this will return a new spreadsheet
      s = Spreadsheet(file); 
      // Make Spreadsheet use a const char* instead

      // Spreadsheet is now an active spreadsheet
      open_spreadsheets.push_back(file);

      // Indicate the connection between socket and the spreadsheet
      sprd_connections.insert(std::pair<int, Spreadsheet>(client_socket, s));
      ***/
    }

  sendConnected(client_socket, numcells);

  /***  // Send all non-empty cells and their contents to the client
  for (map<string, string>::iterator it = s.cells.begin(); it != s.cells.end(); it++)
    {
      string cell = it->first;
      string content = it->second;

      sendCell(client_socket, cell, content);
      } ***/

  // Send the cells
  sendCell(client_socket, "A1", "This");
  sendCell(client_socket, "A2", "is");
  sendCell(client_socket, "A3", "a");
  sendCell(client_socket, "A4", "preloaded");
  sendCell(client_socket, "A5", "spreadsheet");
  sendCell(client_socket, "A6", "=3*3");

  // Start accepting commands
  commandReceived(client_socket);
  
}

/**
 *
 *
 *
 */
void SpreadsheetServer::messageReceived(int client_socket)
{
  std::string line;
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
  /*** look into using stringstream to delimit by ' ' rather than whitespace ***/
  cout << line << endl;
  std::stringstream ss(line);
  std::string token;
  while (ss >> token)
    tokens.push_back(token);
  
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

      // register received
    }
  else if (command.compare("cell") == 0)
    {
      cout << "cell received" << endl;

      // cell command
    }
  else if (command.compare("undo") == 0)
    {
      cout << "undo received" << endl;

      // undo command
    }
  else
    {
      // unrecognized command
    }

  messageReceived(client_socket);
}

void SpreadsheetServer::connectReceived(int client_socket, std::vector<std::string> tokens)
{
  std::string user, filename;

  if (tokens.size() != 3)
    sendError(client_socket, 2, "Incorrect number of tokens");

  user = tokens.at(1);
  filename = tokens.at(2);

  // Not registered
  if (registered_users.find(user) == registered_users.end())
      sendError(client_socket, 4, "Username is not registered or is taken");

  openSpreadsheet(client_socket, filename);

}

void SpreadsheetServer::registerReceived(int client_socket, std::vector<std::string> tokens)
{
  std::string username;

  if (tokens.size() != 2)
    sendError(client_socket, 2, "Incorrect number of tokens");

  username = tokens.at(1);



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
