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
#include <algorithm>
#include <thread>
#include <mutex>

class SpreadsheetServer
{
 public:
  SpreadsheetServer();
  SpreadsheetServer(int port);
  ~SpreadsheetServer();

  void start();

  // <Key, value>: <Socket, Spreadsheet>
  std::map<int, const char*> sprd_connections;


  std::vector<Spreadsheet*> open_spreadsheets;

  // Registered users
  std::set<std::string> registered_users;

  int port;

 private:


  void messageReceived(int client_socket); // This is used to determine what message is sent
  void connectReceived(int client_socket, std::vector<std::string> tokens); // When a connect message is sent this handles that
  void registerReceived(int client_socket, std::vector<std::string> tokens); // Handles register messages which register user
  void cellReceived(int client_socket, std::vector<std::string> tokens); // Handles cell commands to the server finds spreadsheet and changes spreadsheet
  void undoReceived(int client_socket, std::vector<std::string> tokens); // Handles undo messages
  void openSpreadsheet(int client_socket, std::string filename); // helper method used to open a spreadsheet

  bool save_users(); 
  bool load_users();

  void sendConnected(int client_socket, int numcells);
  void sendError(int client_socket, int error_num, std::string info);
  void sendCell(int client_socket, std::string cell_name, std::string contents);

  int server_socket;
  struct sockaddr_in server_addr;
  std::mutex connections_lock, spreadsheets_lock, users_lock;

};

#endif
