/*
 * Filename:Spreadsheet.cpp
 * Author:Ty-Rack-US Park
 * Last Modified: 4/14/2015
 * Version 1.0
 */


#include "Spreadsheet.h"
#include<boost/regex.hpp>
#include<boost/algorithm/string.hpp>


using namespace boost;
using namespace std;

Spreadsheet::Spreadsheet()
{
}

Spreadsheet::Spreadsheet(const Spreadsheet &other)
{
  filename = other.filename;
  sockets = other.sockets;
  graph = other.graph;
  cells = other.cells;
  undo_stack = other.undo_stack;
}

/*
* Reads from file provided. Loads the given spreadsheet if it exists. 
* If not, makes a new one.
*/
Spreadsheet::Spreadsheet(const char* fname)
{
  string line;
  filename = fname; 
  ifstream sprdfile (filename);
  // filename = fname; RAJUL YOU HAVE TO DECLARE IT BEFORE USING IT
  circular = 11;
 
  

  if (sprdfile.is_open())
    {
      while (getline (sprdfile,line) )
	{
	  std::string cellname, contents;
	  // <cellname> <contents>\n
	  stringstream ss(line);
	  
	  ss>>cellname;

	  string c;
	  while(ss>>c)
	    contents= contents+c+" ";
	  
	  contents = contents.substr(0, contents.size()-1);

	  //	  cells[cellname] = contents; // ADD THE NEWB INTO IT RAJUL YOU NEWB
	  setCell(cellname, contents);

	}
      sprdfile.close();
    }
  else
    {
      cout << "Spreadsheet does not exist on server yet" << endl; 
            
    }
}

bool Spreadsheet::setCell(std::string name, std::string contents)
{
  try
    {
      if(cells.at(name).compare(contents) == 0)	
	return true;	
    } 
  catch (const std::out_of_range& oor)
    {      
    }

  //regex e("^[a-zA-Z_]+[a-zA-Z0-9_]*$");
 
  if(contents.substr(0,1).compare("=")==0)	    
    {
      string content_formula = contents.substr(1);
      string token;
      vector<string> variables;
      vector<string> temp;
      vector<string> dependees_backup;
      vector<string> dependents;

      //      dependents.push_back(name);
      dependees_backup = graph.GetDependees(name);
      //      cmatch test;

      string delim("+-/*");
      boost::split(temp, content_formula, boost::is_any_of(delim));
      
      for(int i=0; i<temp.size(); i++)
	{	  
	  // If first character is a letter it is a variable
	  token = temp.at(i);
	  char c = token[0];
	  if ((c >= 65 && c <= 90) || (c >= 97 && c <= 122))	    
	    {
	      variables.push_back(token);
	    }
	}
            
      graph.ReplaceDependees(name, variables);
      
      try
	{
	  getCellsToRecalculate(name);
	}
      catch(int i)
	{
	  if(i==circular)
	    {
	      cout << "Circular dependency!" << endl;
	      // SEG FAULT HERE
	      graph.ReplaceDependees(name, dependees_backup);
	      return false;
	    }
	}
    }

  map<string, string>::iterator it = cells.find(name);
  string prev_contents;
  if (it != cells.end())
      prev_contents = it->second;
  else
    prev_contents = "";

  if(contents.compare("")==0)
    cells.erase(name);
  else
    {
      std::pair<std::map<string,string>::iterator, bool> ret;
      ret = cells.insert ( std::pair<std::string,std::string>(name, contents));
      if(ret.second == false)
	{
	  cells.erase(name);
	  cells.insert ( std::pair<std::string,std::string>(name, contents));
	}
    }
  undo_stack.push_back(pair<string, string>(name, prev_contents));
  saveFile();
  return true;
}

std::pair<std::string, std::string> Spreadsheet::undo()
{
  // Empty return empty list
  if (undo_stack.size() == 0)
    return std::pair<std::string, std::string>("ERROR", "ERROR");
  else
    {
      pair<string, string> prev;
      string name, contents;
      prev = undo_stack.back();
      name = prev.first;
      contents = prev.second;
      std::pair<std::string,std::string> undo = pair<string,string>(name, contents);
      undo_stack.erase(undo_stack.end()-1);

      if(contents.compare("")==0)
	cells.erase(name);
      else
	{
	  std::pair<std::map<string,string>::iterator, bool> ret;
	  ret = cells.insert (undo);
	  if(ret.second == false)
	    {
	      cells.erase(name);
	      cells.insert(undo);
	    }
	}
      saveFile();
      return undo;
    }


}

std::set<std::string> Spreadsheet::getCellsToRecalculate(std::set<std::string> names)
{
  
  std::set<std::string> visited;
  std::set<std::string> changed;
  for(std::set<std::string>::iterator it = names.begin(); it != names.end(); ++it) { 
    const bool vis = visited.find(*it) != visited.end();
     if(!vis){
       visit(*it, *it, visited, changed);
     }
  }
  return changed;

  
}

std::set<std::string> Spreadsheet::getCellsToRecalculate(std::string name)
{
  std::set<std::string> s;
  s.insert(name);
  return getCellsToRecalculate(s); 
}

void Spreadsheet::visit(std::string start,  std::string name, std::set<std::string> visited, std::set<std::string> changed)
{
  visited.insert(name);
  std::vector<std::string> d  = graph.GetDependents(name);
  for(std::vector<std::string>::iterator it = d.begin(); it != d.end(); ++it) {
    const bool vis = visited.find(*it) != visited.end();
    if(*it == start){
      throw circular;
      //Exception Circular

    } else if(!vis){
      visit(start, *it, visited, changed);
    }
  }

  changed.insert(changed.begin(), name);
}

bool Spreadsheet::saveFile(){
  //cells["B5"] = "time";
   //cells["R5"] = "768";

  ofstream myfile (filename);
  if(myfile.is_open()){
    for(std::map<string, string>::iterator it = cells.begin(); it != cells.end(); ++it){
      myfile << it->first + " " + it->second + "\n"; 
    }
    myfile.close();
    return true;
  }
  else {
    // File didn't open
    cout << "Unable to open file" << endl;
    return false;
  }
  

}

