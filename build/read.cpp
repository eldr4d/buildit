#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <string>
#include <utility>
#include "logmanager.hpp"
using namespace std;

typedef struct{
	int employer = -1;
	string token;
	bool HTML = false;
	bool state = false;
	bool rooms = false;
	vector<pair<string,bool> > names;
	string logFile;
	bool allOk = false;
	bool alpha = false;
	bool beta = false;
	int lower = -1;
	int upper = -1;
	int lower2 = -1;
	int upper2 = -1;
	bool timeFlag = false;
	bool roomHistory = false;
}arguments;

arguments getArguments(int argc, char **argv){
	arguments args;
	int c;
	bool firstUdone = false;
	bool firstLdone = false;
	while((c = getopt(argc, argv, "HK:SRE:G:AL:U:TBI")) != -1){
		switch(c){
			case 'K':
				args.token = string(optarg);
				break;
			case 'E':
				args.employer = 1;
				{
					pair<string,bool> tmp;
					tmp.first = string(optarg);
					tmp.second = true;
					args.names.push_back(tmp);
				}
				break;
			case 'G':
				args.employer = 0; 
				{
					pair<string,bool> tmp;
					tmp.first = string(optarg);
					tmp.second = false;
					args.names.push_back(tmp);
				}
				break;
			case 'H':
				args.HTML = true;
				break;
			case 'A':
				args.alpha = true;
				break;
			case 'B':
				args.beta = true;
				break;
			case 'I':
				args.roomHistory = true;
				break;
			case 'U':
				if(firstUdone == false){
					args.upper = atoi(optarg);
					firstUdone = true;
				}else{
					if(args.upper2 == -1){
						args.upper2 = atoi(optarg);
					}else{
						args.allOk = false;
						return args;
					}
				}
				break;
			case 'L':
				if(firstLdone == false){
					args.lower = atoi(optarg);
					firstLdone = true;
				}else{
					if(args.lower2 == -1){
						args.lower2 = atoi(optarg);
					}else{
						args.allOk = false;
						return args;
					}
				}				break;
			case 'S':
				args.state = true;
				break;
			case 'R':
				args.rooms = true;
				break;
			case 'T':
				args.timeFlag = true;
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
		return -1;
	}

	Logmanager myLog(args.logFile, args.token);
	if(myLog.securityViolation){
		cerr << "integrity violation" << endl;
		return -1;
	}
	//myLog.prettyPrint();

	if(args.roomHistory == false && args.timeFlag == false && args.state == true && args.names.size() == 0 && args.rooms == false && args.alpha == false && args.lower == -1 && args.upper == -1 && args.beta == false){
		myLog.printState(args.HTML);
	}else if(args.roomHistory == false && args.timeFlag == false && args.state == false && args.names.size() == 1 && args.rooms == true && args.alpha == false && args.lower == -1 && args.upper == -1 && args.beta == false){
		myLog.printUserData(args.names[0].first, args.employer == 1 ? true : false, args.HTML);
	}else if(args.roomHistory == false && args.timeFlag == false && args.alpha == true && args.lower2 == -1 && args.upper2 == -1 && args.lower >= 0 && args.upper >= 0 && args.upper > args.lower && args.state == false && args.employer == -1 && args.rooms == false && args.beta == false){
		myLog.personsInTimeWindow(args.lower,args.upper,args.HTML);
	}else if(args.roomHistory == false && args.timeFlag == true && args.state == false  && args.names.size() == 1 && args.rooms == false && args.alpha == false && args.lower == -1 && args.upper == -1 && args.HTML == false && args.beta == false){
		myLog.totalTimeOfUser(args.names[0].first, args.employer == 1 ? true : false);
	}else if(args.roomHistory == false && args.timeFlag == false && args.alpha == false && args.lower2 >= 0 && args.upper2 >= 0 && args.lower2 < args.upper2 && args.lower >= 0 && args.upper >= 0 && args.upper > args.lower && args.state == false && args.employer == -1 && args.rooms == false && args.beta == true){
		myLog.leavedPersonsDuringTimeWindow(args.lower,args.upper,args.lower2,args.upper2,args.HTML);
	}else if(args.roomHistory == true && args.timeFlag == false && args.alpha == false && args.lower == -1 && args.upper == -1 && args.state == false && args.names.size() > 0 && args.rooms == false && args.beta == false){
		myLog.printSameRooms(args.names,args.HTML);
	}else{
		cout << "invalid" << endl;
		return -1;
	}
	return 0;
}