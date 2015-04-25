/*
 * Filename:Dependency.h
 * Author:Ty-Rack-US Park
 * Last Modified: 4/8/2015
 * Version: 1.0
 */
#ifndef DEPENDENCYGRAPH_H
#define DEPENDENCYGRAPH_H

#include <map>
#include <string>
#include <sstream>
#include <vector>
#include <set>


/*
 * This class represents a data structure for representing different
 * cells in a spreadsheet and the relations of those cells towards each other. 
 */
class DependencyGraph {

 public:
  DependencyGraph(); // Default Constructor Empty Dependency Graph
  DependencyGraph(const DependencyGraph &other); // Copy constructor
  ~DependencyGraph();

 int size();
 std::vector<std::string> GetDependents(std::string s);
 std::vector<std::string> GetDependees(std::string s);
 bool HasDependents(std::string s);
 bool HasDependees(std::string s);
 void AddDependency(std::string s, std::string t);
 void RemoveDependency(std::string s, std::string t);
 void ReplaceDependents(std::string s, std::vector<std::string> newDependents);
 void ReplaceDependees(std::string s, std::vector<std::string> newDependees);
 void PrintMap();

 private:
  int PairCount;
  std::map<std::string, std::vector<std::string> > key_to_dependents;
  std::map<std::string, std::vector<std::string> > key_to_dependees;
  
};

 #endif

