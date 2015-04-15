/*
 * Filename:Spreadsheet.cpp
 * Author:Ty-Rack-US Park
 * Last Modified: 4/14/2015
 * Version 1.0
 */


#include "Spreadsheet.h"



using namespace std;

Spreadsheet::Spreadsheet()
{
}

/*
* Reads from file provided. Loads the given spreadsheet if it exists. 
* If not, makes a new one.
*/
Spreadsheet::Spreadsheet(const char* fname)
{
  string line;
  ifstream sprdfile (filename);
  filename = fname;
  std::string cellname, contents;
  

  if (sprdfile.is_open())
    {
      while (getline (sprdfile,line) )
	{
	  // <cellname> <contents>\n
	  stringstream ss(line);
	  
	  ss>>cellname;
	  string c;
	 
	  while(ss>>c)
	    contents= contents+c+" ";
	  
	  contents = contents.substr(0, contents.size()-1);

	  
	}
      sprdfile.close();
    }

  else
    {
      cout << "Unable to open file"; 
      
      
    }
}

std::set<std::string> Spreadsheet::setCell(std::string name, std::string contents)
{

  if(contents.substr(0,1).compare("=")==1)	    
    {
      stringstream ss(contents);
      string token;

      while(getline(ss, token, "+-*/")
	{
	  token.
	}
      
      
      contents

    }

      
  cells.insert ( std::pair<std::string,std::string>(cellname, contents));


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



