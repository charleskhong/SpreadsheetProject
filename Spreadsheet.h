/*
 * Filename:Spreadsheet.h
 * Author:Ty-Rack-US Park
 * Last Modified: 4/8/2015
 * Version 1.0
 */

#ifndef SPREADSHEET_H
#define SPREADSHEET_H

#include "DependencyGraph.h"
#include <string.h>
#include <map>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>
#include <mutex>

class Spreadsheet
{
 public:
  Spreadsheet();
  Spreadsheet(const Spreadsheet &other);
  Spreadsheet(const char* fname);
 
  bool setCell(std::string name, std::string contents);
  std::pair<std::string, std::string> undo();
  bool saveFile();


  const char* filename;
  std::vector<int> sockets;
  // Cell name, cell contents
  std::map<std::string, std::string> cells;
  std::vector<std::pair<std::string, std::string> > undo_stack;

  // private:
  DependencyGraph graph;
  int circular;
  std::mutex lock;
  std::set<std::string> getCellsToRecalculate(std::set<std::string> names);
  std::set<std::string> getCellsToRecalculate(std::string name);
  void visit(std::string start, std::string name, std::set<std::string> visited,  std::set<std::string> changed);
  
  };

#endif
