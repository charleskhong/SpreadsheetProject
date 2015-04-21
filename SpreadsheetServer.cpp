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

  ofstream userfile ("usernames");
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
  ifstream userfile ("usernames");
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
	  open_spreadsheets.at(i).sockets.push_back(client_socket);

	  // Spreadsheet make a function to return number of cells
	  numcells = open_spreadsheets.at(i).cells.size();
	  s = open_spreadsheets.at(i);
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

      s.sockets.push_back(client_socket);

      // Spreadsheet is now an active spreadsheet
      spreadsheets_lock.lock();
      open_spreadsheets.push_back(s);
      spreadsheets_lock.unlock();      
    }

  // Indicate the connection between socket and the spreadsheet
  connections_lock.lock();
  sprd_connections.insert(std::pair<int, const char*>(client_socket, file));
  connections_lock.unlock();

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
  else
    {
      cout << "client disconnected" << endl;

      connections_lock.lock();
      const char * filename = sprd_connections.find(client_socket)->second;
      sprd_connections.erase(client_socket);
      connections_lock.unlock();

      spreadsheets_lock.lock();
      for (int i = 0; i < open_spreadsheets.size(); i++)
	{
	  if (strcmp(open_spreadsheets.at(i).filename, filename) == 0)
	    {
	      open_spreadsheets.at(i).sockets.erase(find(open_spreadsheets.at(i).sockets.begin(), open_spreadsheets.at(i).sockets.end(), client_socket));
	      if(open_spreadsheets.at(i).sockets.size() == 0)
		{
		  cout << "something" << endl;
		  open_spreadsheets.erase(open_spreadsheets.begin()+i);
		}
	    }
	}
      spreadsheets_lock.unlock();
      return;
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

  // make this not break
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

  // Client has not connected to a spreadsheet
  spreadsheets_lock.lock();
  int con = sprd_connections.count(client_socket);
  spreadsheets_lock.unlock();

  if (con != 1)
    {
      sendError(client_socket, 3, "Cannot perform command before connecting to a spreadsheet");
      return;
    }

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
  save_users();
  users_lock.unlock();

  // User was already registered
  if (ret.second == false)
    sendError(client_socket, 4, "Username already registered");

}

void SpreadsheetServer::cellReceived(int client_socket, std::vector<std::string> tokens)
{
  std::string cell, contents;

  // Client has not connected to a spreadsheet
  connections_lock.lock();
  int con = sprd_connections.count(client_socket);
  connections_lock.unlock();

  if (con != 1)
    {
      sendError(client_socket, 3, "Cannot perform command before connecting to a spreadsheet");
      return;
    }

  //need to check when they empty out a string and send [Cell <callname> ""\n]
  if (tokens.size() != 3)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }
  cell = tokens.at(1);
  contents = tokens.at(2);
  
  connections_lock.lock();
  const char * filename = sprd_connections.find(client_socket)->second;
  connections_lock.unlock();

  spreadsheets_lock.lock();
  Spreadsheet s;
  for (int i = 0; i < open_spreadsheets.size(); i++)
    {
      if (strcmp(filename, open_spreadsheets.at(i).filename) == 0)
	{
	  bool c = open_spreadsheets.at(i).setCell(cell, contents);
	  if (c)
	    {	     
	      s = open_spreadsheets.at(i);
	      vector<int> sockets = s.sockets;

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
  // Client has not connected to a spreadsheet
  spreadsheets_lock.lock();
  int con = sprd_connections.count(client_socket);
  spreadsheets_lock.unlock();

  if (con != 1)
    {
      sendError(client_socket, 3, "Cannot perform command before connecting to a spreadsheet");
      return;
    }

  if (tokens.size() != 1)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      return;
    }

  connections_lock.lock();
  const char* filename = sprd_connections.find(client_socket)->second;
  connections_lock.unlock();

  spreadsheets_lock.lock();
  Spreadsheet s;
  for (int i = 0; i < open_spreadsheets.size(); i++)
    {
      if (strcmp(filename, open_spreadsheets.at(i).filename) == 0)
	{
	  std::pair<std::string, std::string> change;
	  change = open_spreadsheets.at(i).undo();
	  s = open_spreadsheets.at(i);

	  string name, contents;
	  name = change.first;
	  contents = change.second;

	  if (name.compare("ERROR") != 0)
	    {
	      vector<int> sockets = s.sockets;
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
