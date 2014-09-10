#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <fstream> 
#include "logmanager.hpp"
using namespace std;

typedef struct{
	int timestamp = -1;
	int employer = -1;
	string token;
	int arrival = -1;
	int room = -1;
	bool batchMode = false;
	string batchFileName;
	string name;
	string logFile;
	bool allOk = false;
}arguments;

arguments getArguments(int argc, char **argv){
	arguments args;
	int c;
	optind = 0;
	/*cout << argc << ":: ";
	for(int i = 0; i<argc; i++){
		cout << argv[i] << ", ";
	}
	cout << endl;*/

	while((c = getopt(argc, argv, "T:K:E:G:ALR:B:")) != -1){
		switch(c){
			case 'T':
				args.timestamp = atoi(optarg);
				if(args.timestamp < 0){
					args.allOk = false;
					return args;
				}
				break;
			case 'K':
				args.token = string(optarg);
				break;
			case 'E':
				if(args.employer == -1){
					args.employer = 1;
					args.name = string(optarg);
				}else{
					args.allOk = false;
					return args;
				}
				break;
			case 'G':
				if(args.employer == -1){
					args.employer = 0; 
					args.name = string(optarg);
				}else{
					args.allOk = false;
					return args;
				}
				break;
			case 'A':
				if(args.arrival != -1){
					args.allOk = false;
					return args;
				}
				args.arrival = 1;
				break;
			case 'L':
				if(args.arrival != -1){
					args.allOk = false;
					return args;
				}
				args.arrival = 0;
				break;
			case 'R':
				args.room = atoi(optarg);
				break;
			case 'B':
				args.batchMode = true;
				args.batchFileName = string(optarg); 
				if(argc != 3){
					args.allOk = false;
					return args;
				}else{
					args.allOk = true;
					return args;
				}
				break;
			case '?':
				args.allOk = false;
				return args;
				break;
			default:
				args.allOk = false;
				return args;
				cout << "Dafuq" << endl;
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

int handleInput(arguments args){
	if(args.token.length() <= 0 || args.timestamp == -1  || args.employer == -1 || args.arrival == -1){
		cout << "invalid" << endl;
		return -1;
	}
	Logmanager myLog(args.logFile, args.token);
	if(myLog.securityViolation){
		cerr << "security error" << endl;
		return -1;
	}
	if(myLog.append(args.name, args.timestamp, args.employer == 1 ? true : false, args.room, args.arrival == 1? true : false) < 0 && args.batchMode == false){
		cout << "invalid" << endl;
		return -1;
	}else if(args.batchMode == true){
		return 0;
	}
	myLog.serialize();
	return 0;
}

int main(int argc, char **argv){

    string tempFileName = "ProgramName";
	arguments args = getArguments(argc, argv);
	if(args.allOk == false){
		cout << "invalid" << endl;
		return -1;
	}
	int returnState = 0;
	if(args.batchMode == true){
    	ifstream batchFile(args.batchFileName);
    	if(batchFile.good()){
    		while(batchFile.eof() == false){
    			char line[512];
    			batchFile.getline(line,512);
    			char *point = strtok(line," ");
    			char *newargv[40];
    			int newargc = 1;
    			newargv[0] = (char *)tempFileName.c_str();
    			while(point != NULL){
    				newargv[newargc] = point;
    				newargc++;
    				point = strtok(NULL, " ");
    			}
	    		arguments args2 = getArguments(newargc, newargv);
	    		if(args2.allOk == false || args2.batchMode == true){
	    			continue;
	    		}
	    		returnState = handleInput(args2);
    		}
    	}else{
    		batchFile.close();
    		return 0;
    	}
	}else{
		returnState = handleInput(args);
	}
	return returnState;
}