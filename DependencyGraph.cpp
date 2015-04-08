/*
 * Filename:Dependency.c
 * Author:Ty-Rack-US Park
 * Last Modified: 4/8/2015
 * Version: 1.0
 */

#include "DependencyGraph.h"

#include <map>
#include <string>
#include <sstream>
#include <vector>


using namespace std;

DependencyGraph::DependencyGraph(){

}

DependencyGraph::DependencyGraph(const DependencyGraph &other){
}

DependencyGraph::~DependencyGraph(){}


bool DependencyGraph::HasDependents(string s){
}

bool  DependencyGraph::HasDependees(string s){
}

void DependencyGraph::AddDependency(string s, string t){
}


void DependencyGraph::RemoveDependency(string s, string t){
}

void DependencyGraph::ReplaceDependents(string s, vector<string> newDependents){

}

void DependencyGraph::ReplaceDependees(string s, vector<string> newDependees){
}
