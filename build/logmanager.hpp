#include <map>
#include <utility>
#include <string>
#include <iostream>
#include <vector>

#include <boost/serialization/serialization.hpp>
using namespace std;

class visit{
public:
	bool employer;
	bool arrival;
	int room;
	int timeStamp;
	int upperTime;
	int lowerTime;
private:
	friend class boost::serialization::access;
    // When the class Archive corresponds to an output archive, the
    // & operator is defined similar to <<.  Likewise, when the class Archive
    // is a type of input archive the & operator is defined similar to >>.
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & employer;
        ar & arrival;
        ar & room;
        ar & timeStamp;
        ar & upperTime;
        ar & lowerTime;
    };
};


class Logmanager{
private:
	int currMaxTime;
	string logFileName;
	string secret;
	std::map<string,vector<visit> > artlog;
public:
	bool securityViolation;
public:
	Logmanager(string filename, string token):currMaxTime(-1),logFileName(filename),secret(token),securityViolation(false){
		try{
			deserialize();
			for(map<string, vector<visit> >::iterator iter = artlog.begin(); iter != artlog.end(); iter++){
				if(iter->second.front().timeStamp > currMaxTime){
					currMaxTime = iter->second.front().timeStamp;
				}
			}
			//prettyPrint();
		}catch(const int& e){
			securityViolation = true;
		}
	};


	void serialize();
	void deserialize();
	int append(string name, int timestamp, bool employer, int room, bool arrival);
	void prettyPrint();
	void printState(bool toHtml);
	void printUserData(string user, bool toHtml);
	void totalTimeOfUser(string user);
	void personsInTimeWindow(int lower, int upper, bool toHtml);

};

