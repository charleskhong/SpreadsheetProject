/*
 * Filename:SpreadsheetServer.h
 * Author:Ty-Rack-US Park
 * Last Modified: 4/8/2015
 * Version 1.0
 */

#ifndef SPREADSHEETSERVER_H
#define SPREADSHEETSERVER_H

#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <map>
#include <vector>
#include <set>
#include "Spreadsheet.h"
#include "User.h"
#include <boost/thread.hpp>

class SpreadsheetServer
{
 public:
  SpreadsheetServer();
  SpreadsheetServer(int port);
  ~SpreadsheetServer();

  /*
   * Create the socket and begin listening for connections
   * When a connection is made, call ConnectionReceived 
   *  on a separate thread/process
   */
  void start();


  // <Key, value>: <Socket, Spreadsheet>
  std::map<int, Spreadsheet> active_spreadsheets;

  std::vector<User> active_users;

  // Active spreadsheets
  // ? Load all spreadsheets up front or go to disk when needed
  // Con: have to constantly go through large vector
  //  use resources/memory unnecessarily
  std::vector<Spreadsheet> spreadsheetList;

  // Registered users
  std::set<std::string> userList;

  int port;

 private:

  /*
   * Check to see if the user is valid and is registered
   *  if not, send an error
   * 
   * OpenSpreadsheet
   * Listen for commands
   *
   */
  void connectionReceived(int client_socket);

  /*
   * Check to see if the spreadsheet exists
   * add to data structures (even if old or new)
   * 
   * Send confirmation 
   * Iterate through all cells and send the contents
   * 
   */
  void openSpreadsheet(int client_socket, std::string filename);

  void sendConnected(int client_socket, int numcells);
  void sendError(int client_socket, int error_num, std::string info);

  /*
   * Construct the send command to client using the given parameters
   * then send it to the client
   */
  void sendCell(int client_socket, std::string cell_name, std::string contents);
  // repeat this for the other commands


  void commandReceived(int client_socket);

  int server_socket;
  struct sockaddr_in server_addr;

};

#endif
