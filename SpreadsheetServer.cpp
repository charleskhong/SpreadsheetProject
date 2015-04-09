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

  userList.insert("sysadmin");

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

      std::thread client (connectionReceived, client_socket);

      // pid = fork();
      /*      if (pid < 0)
	error("ERROR on fork");

      // Child process
      else if (pid == 0)
	{
	  close(server_socket);
	  connectionReceived(client_socket);
	  //dostuff(client_socket);
	  exit(0);
	}

      // Main process
      else
	{
	  close(client_socket);
	}*/
    }

  close(server_socket);
}

void SpreadsheetServer::connectionReceived(int client_socket)
{
  int n;
  char buffer[256];
  char * token;
  std::vector<char*> tokens;

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
      cout << "Current token: " << token << endl;
      tokens.push_back(token);
      token = strtok(NULL, " ");
    }

  // Incorrect number of tokens
  if (tokens.size() != 3)
    {
      sendError(client_socket, 2, "Incorrect number of tokens");
      connectionReceived(client_socket);
    }
    
  // First token is not connect
  if (strcmp(tokens.at(0), "connect") != 0)
    {
      sendError(client_socket, 2, "Expected connect command");
      connectionReceived(client_socket);
    }

  if (userList.find(tokens.at(1)) == userList.end())
    {
      sendError(client_socket, 4, "Username is not registered or is taken");
      connectionReceived(client_socket);
	    
    }

  if (userList.find(tokens.at(1)) != userList.end())
    {
      cout << active_users.size() << endl;
      // Loop through active_users
      for (int i = 0; i < active_users.size(); i++)
	{
	  char* name = active_users.at(i).username; 
	  if (strcmp(name, tokens.at(1)) == 0)
	    {
	      sendError(client_socket, 4, "Username is not registered or is taken");
	      connectionReceived(client_socket);
	    }
	}
    }

  cout << "Username was not taken" << endl;
  User newuser (tokens.at(1), client_socket);
  active_users.push_back(newuser);
  cout <<  active_users.size() << endl;
  n = write(client_socket, "I got your message\n", 20);

  for (int i = 0; i < active_users.size(); i++)
    {
      cout << active_users.at(i).username << endl;
    }
    
  // ? Close the connection at this point ?
  if (n < 0)
    error("ERROR writing to socket");

    
}

void SpreadsheetServer::commandReceived(int client_socket)
{
}

void SpreadsheetServer::openSpreadsheet(int client_socket, std::string filename)
{
}

void SpreadsheetServer::sendConnected(int client_socket, int numcells)
{
}

void SpreadsheetServer::sendCell(int client_socket, std::string cell_name, std::string contents)
{
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



void dostuff (int client_socket)
{
  int n;
  char buffer[256];

  bzero(buffer, 256);
  n = read(client_socket, buffer, 256);

  while (n > 0)
    {
      if (n < 0)
	error("ERROR reading from socket");

      printf("Here is the message: %s\n", buffer);
      n = write(client_socket, "I got your message\n", 20);
      if (n < 0)
	error("ERROR writing to socket");

      bzero(buffer, 256);
      n = read(client_socket, buffer, 256);

    }
}
