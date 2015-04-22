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
#include <algorithm>
#include <thread>
#include <mutex>

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
  std::map<int, const char*> sprd_connections;

  // Active spreadsheets
  // ? Load all spreadsheets up front or go to disk when needed
  // Con: have to constantly go through large vector
  //  use resources/memory unnecessarily
  std::vector<Spreadsheet> open_spreadsheets;

  // Registered users
  std::set<std::string> registered_users;

  int port;

 private:


  void messageReceived(int client_socket);
  void connectReceived(int client_socket, std::vector<std::string> tokens);
  void registerReceived(int client_socket, std::vector<std::string> tokens);
  void cellReceived(int client_socket, std::vector<std::string> tokens);
  void undoReceived(int client_socket, std::vector<std::string> tokens);

  bool save_users();
  bool load_users();
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

  int server_socket;
  struct sockaddr_in server_addr;
  std::mutex connections_lock, spreadsheets_lock, users_lock;

};

#endif
