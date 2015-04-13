# Authors: Ty-Rack-US Park
# Date: April 13, 2015
#
# Description: Make for spreadsheet server

all: server spreadsheet dependency user
	g++ SpreadsheetServer.o Spreadsheet.o DependencyGraph.o User.o /usr/local/lib/libboost_system.a

server: SpreadsheetServer.cpp SpreadsheetServer.h 
	g++ -c SpreadsheetServer.cpp

spreadsheet: Spreadsheet.cpp Spreadsheet.h 
	g++ -c Spreadsheet.cpp

dependency: DependencyGraph.cpp DependencyGraph.h 
	g++ -c DependencyGraph.cpp

user: User.cpp User.h
	g++ -c User.cpp

clean:
	rm -f ./a.out *.o