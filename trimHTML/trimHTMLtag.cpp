#include <iostream>
#include <fstream>
#include <string>
#include <string.h>

using namespace std;


int main(int argc, char ** argv) {
	string fileName=argv[1];
	string pattern =argv[2];
	string beginP="<";
	beginP+=pattern;
	string endP="/";
	endP+=pattern+">";

  
   int length;
  char * buffer;
  char * outBuffer;

  ifstream is(fileName.c_str());

  // get length of file:
  is.seekg (0, ios::end);
  length = is.tellg();
  is.seekg (0, ios::beg);

  // allocate memory:
  buffer = new char [length];
  outBuffer=new char[length];

  // read data as a block:
  is.read (buffer,length);
  is.close();

char * curr=buffer;
char * currOut=outBuffer;

  while (true) {
  	
  	char * needle=strstr(curr,beginP.c_str());
  	if (needle==NULL) {
  		strcpy(currOut, curr);
  		break;
  	}
  	int delta=needle-curr;
  	strncpy(currOut, curr,delta);
  	currOut+=delta;
  	char * close=strstr(needle+1,endP.c_str());
  	if (close==NULL) {
  		cerr<<"Error: Found Open no close at "<<needle-buffer<<"\n";
  		curr=needle+beginP.size()+1;
  		//return 1;
  	} else
  		curr=close+endP.size()/*+1*/;		
  }
	cout<<outBuffer;
  delete[] buffer;
  delete[] outBuffer;
  return 0;
}

