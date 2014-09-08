#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include "logmanager.hpp"
using namespace std;

typedef struct{
	int employer = -1;
	string token;
	bool HTML = false;
	bool state = false;
	bool rooms = false;
	string name;
	string logFile;
	bool allOk = false;
}arguments;

arguments getArguments(int argc, char **argv){
	arguments args;
	int c;
	while((c = getopt(argc, argv, "HK:SRE:G:")) != -1){
		switch(c){
			case 'K':
				args.token = string(optarg);
				break;
			case 'E':
				if(args.employer == -1){
					args.employer = 1;
					args.name = string(optarg);
					args.name.append("E");
				}else{
					args.allOk = false;
					return args;
				}
				break;
			case 'G':
				if(args.employer == -1){
					args.employer = 0; 
					args.name = string(optarg);
					args.name.append("G");
				}else{
					args.allOk = false;
					return args;
				}
				break;
			case 'H':
				args.HTML = true;
				break;
			case 'S':
				args.state = true;
				break;
			case 'R':
				args.rooms = true;
				break;
			case '?':
				break;
		}
	}

	args.allOk = false;
	for(; optind < argc; optind++){
		if(args.allOk == true){
			args.allOk = false;
			return args;
		}else{
			args.allOk = true;
		}
		args.logFile = string(argv[optind]);
	}
	return args;
}

int main(int argc, char **argv){

	arguments args = getArguments(argc, argv);
	if(args.allOk == false || args.token.length() <= 0){
		cout << "invalid" << endl;
		return 0;
	}
	Logmanager myLog(args.logFile, args.token);
	
	myLog.prettyPrint();

	if(args.state == true && args.employer == -1 && args.rooms == false){
		myLog.printState();
	}
	if(args.state == false && args.employer != -1 && args.rooms == true){
		myLog.printUserData(args.name);
	}
	return 0;
}