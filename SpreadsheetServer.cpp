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
  int server_socket, client_socket, port, pid;
  socklen_t client_length;
  struct sockaddr_in server_addr, client_addr;

  if (argc < 2)
    {
      cout << "Please provide a port number for the server" << endl;
      return 0;
    }


  // Load all spreadsheets and users

  server_socket = socket(AF_INET, SOCK_STREAM, 0);
  if (server_socket < 0)
    error("ERROR opening socket");

  bzero((char *) &server_addr, sizeof(server_addr));
  port = atoi(argv[1]);
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
    error("ERROR on binding");

  listen(server_socket, 5);
  client_length = sizeof(client_addr);

  while (1)
    {
      client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_length);
      if (client_socket < 0)
	error("ERROR on accept");

      pid = fork();
      if (pid < 0)
	error("ERROR on fork");

      // Child process
      else if (pid == 0)
	{
	  close(server_socket);
	  // Connection received
	  // Process the socket connection and listen for commands
	  // ConnectionReceived();
	  dostuff(client_socket);
	  exit(0);
	}

      // Main process
      else
	{
	  close(client_socket);
	}
    }

  close(server_socket);
  return 0;
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
