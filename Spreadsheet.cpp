/*
 * Filename:Spreadsheet.cpp
 * Author:Ty-Rack-US Park
 * Last Modified: 4/14/2015
 * Version 1.0
 */


#include "Spreadsheet.h"
#include <fstream>
#include <set>

Spreadsheet::Spreadsheet()
{
}

/*
* Reads from file provided. Loads the given spreadsheet if it exists. 
* If not, makes a new one.
*/
Spreadsheet::Spreadsheet(char* filename)
{
  /*
  string line;
  ifstream sprdfile (filename);
  
  std::string cellname, contents;


  if (sprdfile.is_open())
    {
      while ( getline (sprdfile,line) )
	{
	  // <cellname> <contents> \n
	  std::vector<char*> words;
	  char* chars_array = strtok(filename, " ");
	  while(chars_array!=NULL)
	    {
	      words.push_back(chars_array);
	      chars_array = strtok(NULL, " ");
	    }
      
	  cellname(words.at(0));
	  contents(words.at(1));
	  cells.insert ( std::pair<std::string,std::string>(cellname, contents));
	}
      sprdfile.close();
    }

  else
    cout << "Unable to open file"; 
  */
}

std::set<std::string> Spreadsheet::getCellsToRecalculate(std::set<std::string> names)
{
  
  std::set<std::string> visited;
  std::vector<std::string> changed;
  for(std::set<std::string>::iterator it = names.begin(); it != names.end(); ++it) { 
    const bool vis = visited.find(*it) != visited.end();
     if(vis){
       visit(*it, *it, visited, changed);
     }
  }
  
}

std::set<std::string> Spreadsheet::getCellsToRecalculate(std::string name)
{
  std::set<std::string> s;
  s.insert(name);
  return getCellsToRecalculate(s); 
}

void Spreadsheet::visit(std::string start,  std::string name, std::set<std::string> visited, std::vector<std::string> changed)
{
  visited.insert(name);
  std::vector<std::string> d  = graph.GetDependents(name);
  for(std::vector<std::string>::iterator it = d.begin(); it != d.end(); ++it) {
    const bool vis = visited.find(*it) != visited.end();
    if(*it == start){
      //Exception Circular

    } else if(!vis){
      visit(start, *it, visited, changed);
    }
  }

  changed.insert(changed.begin(), name);
}



