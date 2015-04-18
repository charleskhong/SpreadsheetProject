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

	  cells[cellname] = contents; // ADD THE NEWB INTO IT RAJUL YOU NEWB

	}
      sprdfile.close();
    }
  else
    {
      cout << "Unable to open file"; 
      
      
    }
}

bool Spreadsheet::setCell(std::string name, std::string contents)
{
  try{
    if(cells.at(name).compare(contents) == 0){
      return false;
    }
  } catch (const std::out_of_range& oor){

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

      dependents.push_back(name);
      dependees_backup = graph.GetDependees(name);
      cmatch test;



      string delim("+-/*");
      boost::split(temp, content_formula, boost::is_any_of(delim));
      
      for(int i=0; i<temp.size(); i++)
	{
	  /*
	  if(boost::regex_match(temp[i],test, e))
	    {
	      variables.push_back(temp[i]);
	      }*/
	}
      graph.ReplaceDependees(name, variables);
      
      try
	{
	  set<string> cells = getCellsToRecalculate(name);
	  
	  for(set<string>::iterator it = cells.begin(); it!=cells.end(); it++)
	    {
	      dependents.push_back(*it);
	    }
	  
	}
      catch(int i)
	{
	  if(i==circular)
	    {
	      graph.ReplaceDependees(name, dependees_backup);
	      return false;
	    }
	}
      
    }
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

 return true;
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
      throw circular;
      //Exception Circular

    } else if(!vis){
      visit(start, *it, visited, changed);
    }
  }

  changed.insert(changed.begin(), name);
}

bool Spreadsheet::saveFile(){
  cells["B5"] = "time";
  cells["R5"] = "768";

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
    cout << "Unable to open file";
    return false;
  }
  

}

