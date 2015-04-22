# Authors: Ty-Rack-US Park
# Date: April 13, 2015
#
# Description: Make for spreadsheet server

all: server spreadsheet dependency
	g++ SpreadsheetServer.o Spreadsheet.o DependencyGraph.o /usr/local/lib/libboost_system.a -lpthread

server: SpreadsheetServer.cpp SpreadsheetServer.h 
	g++ -c SpreadsheetServer.cpp -std=gnu++0x

spreadsheet: Spreadsheet.cpp Spreadsheet.h 
	g++ -c Spreadsheet.cpp

dependency: DependencyGraph.cpp DependencyGraph.h 
	g++ -c DependencyGraph.cpp

clean:
	rm -f ./a.out *.o