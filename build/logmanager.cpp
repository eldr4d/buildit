#include <sstream>
#include <fstream> 
#include <algorithm>
#include <functional>
#include <stdint.h>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "logmanager.hpp"
#include "blowfish.hpp"
#include "printHTML.hpp"

using namespace std;



void Logmanager::serialize(){
 	ofstream file(logFileName.c_str(), ios::out | ios::binary);
 	stringstream ss;
	boost::archive::text_oarchive oarch(ss);
	oarch << artlog;
	string holeFile = ss.str();

	int fileLength = holeFile.length();

	/*int32_t intHash = hash<string>()(holeFile);
	char hashStr[4];
	memcpy(hashStr, (char *)intHash, 4);
	holeFile.insert(0,hashStr,4);
	fileLength += 4;*/

	while(fileLength%8 != 0){
		fileLength++;
		holeFile.append("-");
	}

	//cout << fileLength << endl;
	//cout << holeFile << endl;
	unsigned char *encryptedData1, *encryptedData2;
	encryptedData1 = new unsigned char[fileLength];

	CBlowFish oBlowFish((unsigned char*)secret.c_str(), secret.length());
	oBlowFish.Encrypt((unsigned char*)holeFile.c_str(), encryptedData1, fileLength, CBlowFish::CBC);
	//oBlowFish.Encrypt(encryptedData1, encryptedData2, fileLength, CBlowFish::CBC);
	//cout << encryptedData1 << endl;
	//file << holeFile;
	file.write((const char*)encryptedData1,fileLength);
	file.close();
}

void Logmanager::deserialize(){
	ifstream file(logFileName.c_str(), ios::in | ios::binary);
	if(!file.good()){
		return;
	}

	string holeFile((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	int fileLength = holeFile.length();	
	if(fileLength == 0){
		return;
	}

	unsigned char *unencryptedData;
	unencryptedData = new unsigned char[fileLength+1];
	CBlowFish oBlowFish((unsigned char*)secret.c_str(), secret.length());
	oBlowFish.Decrypt((unsigned char*)holeFile.c_str(), unencryptedData, fileLength, CBlowFish::CBC);
	//oBlowFish.Encrypt(encryptedData1, encryptedData2, fileLength, CBlowFish::CBC);
	unencryptedData[fileLength] = '\0';
	//cout << fileLength << endl;
	//cout << unencryptedData << endl;
	while(unencryptedData[fileLength-1] == '-'){
		unencryptedData[fileLength-1] = '\0';
		fileLength--;
	}

	/*string allData((char *)unencryptedData);
	char hashString[4];
	memcpy(hashString,unencryptedData,4);
	allData.erase(0,4);

	int32_t intHash = hash<string>()(allData);

	if(*(int32_t *)hashString != intHash){
		throw -2;
	}
	
	//cout << unencryptedData << endl;
	stringstream ss;
	ss << allData;*/


	stringstream ss;
	ss << unencryptedData;
	try{
		boost::archive::text_iarchive iarch(ss); 
		iarch >> artlog;
	}catch(...){
		throw -2;
	}
	file.close();
}

int Logmanager::append(string name, int timestamp, bool employer, int room, bool arrival){
	//Wrong timestamp
	if(currMaxTime >= timestamp){
		return -1;
	}
	currMaxTime = timestamp;
	visit newvis;
	newvis.employer = employer;
	newvis.arrival = arrival;
	newvis.room = room;
	newvis.timeStamp = timestamp;
	if(arrival == true && room == -1){
		newvis.lowerTime = timestamp;
		newvis.upperTime = -1;
	}else if(arrival == false && room == -1){
		newvis.upperTime = timestamp;
	}else{
		newvis.upperTime = -1;
	}
	map<string, vector<visit> >::iterator iter = artlog.find(name);
	if(iter != artlog.end() && iter->second.back().employer != employer){
		return -1;
	}
	if((arrival == true && iter == artlog.end() && room == -1) ||
	   (arrival == true && iter != artlog.end() && room != -1 && ((iter->second.back().arrival == false && iter->second.back().room != -1) || (iter->second.back().arrival == true && iter->second.back().room == -1))) ||
	   (arrival == false && iter != artlog.end() && room == -1 && ((iter->second.back().arrival == false && iter->second.back().room !=-1) || (iter->second.back().arrival == true && iter->second.back().room == -1))) ||
	   (arrival == false && iter != artlog.end() && room != -1 && ((iter->second.back().arrival == true && iter->second.back().room == room)))){
	   	if(iter != artlog.end()){
	   		newvis.lowerTime = iter->second.back().lowerTime;
	   	}
		artlog[name].push_back(newvis);
		return 0;
	}else{
		//cout << "T = " << newvis.timeStamp << " A = " << newvis.arrival << " R = " << newvis.room << endl;
		return -1;
	}
}

void Logmanager::prettyPrint(){
	for(map<string, vector<visit> >::iterator iter = artlog.begin(); iter != artlog.end(); iter++){
		string tmp = iter->first;
		cout << "User = " << tmp << endl;
		for(int i=iter->second.size()-1; i>=0; i--){
			cout << "\tEvent " << i << ") timestamp = " << iter->second[i].timeStamp << " employer = "
				<< iter->second[i].employer << " room = " << iter->second[i].room << " arrival = "
				<< iter->second[i].arrival << " [" << iter->second[i].lowerTime << "," << iter->second[i].upperTime << "]" << endl;
		}
	}
}


void Logmanager::printState(bool toHtml){
	HTML htmlprint;
	if(toHtml){
		htmlprint.header();
		htmlprint.startTable();
	}
	vector<string> employersInGallery;
	vector<string> guestsInGallery;
	vector<string> totalPersons;
	for(map<string, vector<visit> >::iterator iter = artlog.begin(); iter != artlog.end(); iter++){
		if(iter->second.back().arrival==false && iter->second.back().room==-1){
			continue;
		}else{
			if(iter->second.back().employer){
				employersInGallery.push_back(iter->first);
			}else{
				guestsInGallery.push_back(iter->first);
			}
			totalPersons.push_back(iter->first);
		}
	}

	bool first;
	if(toHtml){
		htmlprint.addDoubleHeaderToTable("Employee", "Guest");
		int big =  employersInGallery.size() > guestsInGallery.size() ?  employersInGallery.size() : guestsInGallery.size();
		for(unsigned int i=0; i<big; i++){
			string data1, data2;
			if(i < employersInGallery.size()){
				data1 = employersInGallery[i];
			}else{
				data1 = "";
			}
			if(i < guestsInGallery.size()){
				data2 = guestsInGallery[i];
			}else{
				data2 = "";
			}
			htmlprint.addDoubleElementToTable(data1,data2);
		}
		htmlprint.endTable();
		htmlprint.startTable();
	}else{
		first = true;
		for(unsigned int i=0; i<employersInGallery.size(); i++){
			string tmp = employersInGallery[i];
			if(first)
				cout << tmp;
			else
				cout << "," << tmp;
			first = false;
		}
		cout << endl;
		first = true;
		for(unsigned int i=0; i<guestsInGallery.size(); i++){
			string tmp = guestsInGallery[i];
			if(first)
				cout << tmp;
			else
				cout << "," << tmp;
			first = false;
		}
		cout << endl;
	}

	map<int,vector<string> > rooms;

	for(unsigned int i=0; i< totalPersons.size(); i++){
		string temp = totalPersons[i];
		if(artlog[temp].back().arrival==false){
			//They are not in a room
			continue;
		}else{
			int room = artlog[temp].back().room;
			if(room == -1){
				continue;
			}
			rooms[room].push_back(temp);
		}
	}

	if(toHtml){
		htmlprint.addDoubleHeaderToTable("Room ID", "Occupants");
	}
	for(map<int,vector<string> >::iterator iter = rooms.begin(); iter!=rooms.end(); iter++){
		string data1,data2;
		if(toHtml){
			data1 = to_string(iter->first);
		}else{
			cout << iter->first << ": ";
		}
		//sort(iter->second.begin(), iter->second.end());
		if(toHtml){
			first = true;
			for(unsigned int i=0; i<iter->second.size(); i++){
				string tmp = iter->second[i];
				if(first){
					data2 = tmp;
				}else{
					data2.append(",");
					data2.append(tmp);
				}
				first = false;
			}
			htmlprint.addDoubleElementToTable(data1,data2);
		}else{
			first = true;
			for(unsigned int i=0; i<iter->second.size(); i++){
				string tmp = iter->second[i];
				if(first)
					cout << tmp;
				else
					cout << "," << tmp;
				first = false;
			}
			cout << endl;
		}
	}
	if(toHtml){
		htmlprint.endTable();
		htmlprint.footer();
	}
}

void Logmanager::printUserData(string user, bool employer, bool toHtml){
	HTML htmlprint;

	map<string,vector<visit> >::iterator it = artlog.find(user);
	if(it != artlog.end() && it->second.back().employer != employer){
		return;
	}
	if(toHtml){
		htmlprint.header();
		htmlprint.startTable();
		htmlprint.addHeaderToTable("Rooms");
	}
	if(it == artlog.end()){
		if(toHtml){
			htmlprint.endTable();
			htmlprint.footer();
		}
		return;
	}
	vector<visit> allVisits = it->second;
	vector<int> roomsEntered;
	for(int i=allVisits.size()-1; i>=0; i--){
		if(allVisits[i].arrival == true && allVisits[i].room != -1){
			roomsEntered.insert(roomsEntered.begin(),allVisits[i].room);
		}
	}

	bool first = true;
	for(unsigned int i=0; i<roomsEntered.size(); i++){
		if(first){
			if(toHtml){
				htmlprint.addElementToTable(to_string(roomsEntered[i]));
			}else{
				cout << roomsEntered[i];
			}
		}else{
			if(toHtml){
				htmlprint.addElementToTable(to_string(roomsEntered[i]));
			}else{
				cout << "," << roomsEntered[i];
			}
		}
		first = false;
	}
	if(!toHtml){
		cout << endl;
	}
	if(toHtml){
		htmlprint.endTable();
		htmlprint.footer();
	}
}

void Logmanager::personsInTimeWindow(int lower, int upper, bool toHtml){
	HTML htmlprint;
	if(toHtml){
		htmlprint.header();
		htmlprint.startTable();
		htmlprint.addHeaderToTable("Employees");
	}
	vector<string> people;
	for(map<string,vector<visit> >::iterator iter = artlog.begin(); iter != artlog.end(); iter++){
		if(iter->second.back().employer == false){
			continue;
		}
		visit temp = iter->second.back();
		if((temp.upperTime != -1 && temp.upperTime < lower) || temp.lowerTime > upper){
			continue;
		}
		string user = iter->first;
		people.push_back(user);
	}
	if(toHtml){
		for(unsigned int i=0; i<people.size(); i++){
			htmlprint.addElementToTable(people[i]);
		}
	}else{
		bool first = true;
		for(unsigned int i=0; i<people.size(); i++){
			if(first){
				cout << people[i];
			}else{
				cout << "," << people[i];
			}
			first = false;
		}
		cout << endl;
	}
	if(toHtml){
		htmlprint.endTable();
		htmlprint.footer();
	}
}

void Logmanager::totalTimeOfUser(string user, bool employer){
	map<string,vector<visit> >::iterator it = artlog.find(user);
	if(it != artlog.end() && it->second.back().employer != employer){
		return;
	}
	if(it == artlog.end()){
		return;
	}

	if(it->second.back().upperTime == -1){
		cout << currMaxTime - it->second.back().lowerTime << endl;
	}else{
		cout << it->second.back().upperTime - it->second.back().lowerTime << endl;
	}
}

void Logmanager::leavedPersonsDuringTimeWindow(int lower, int upper, int lower2, int upper2, bool toHtml){
	HTML htmlprint;
	if(toHtml){
		htmlprint.header();
		htmlprint.startTable();
		htmlprint.addHeaderToTable("Employees");
	}
	vector<string> people;
	for(map<string,vector<visit> >::iterator iter = artlog.begin(); iter != artlog.end(); iter++){
		if(iter->second.back().employer == false){
			continue;
		}
		visit temp = iter->second.back();
		if((temp.upperTime != -1 && temp.upperTime < lower) || temp.lowerTime > upper){
			continue;
		}
		if(!((temp.upperTime != -1 && temp.upperTime < lower2) || temp.lowerTime > upper2)){
			continue;
		}
		string user = iter->first;
		people.push_back(user);
	}
	if(toHtml){
		for(unsigned int i=0; i<people.size(); i++){
			htmlprint.addElementToTable(people[i]);
		}
	}else{
		bool first = true;
		for(unsigned int i=0; i<people.size(); i++){
			if(first){
				cout << people[i];
			}else{
				cout << "," << people[i];
			}
			first = false;
		}
		cout << endl;
	}
	if(toHtml){
		htmlprint.endTable();
		htmlprint.footer();
	}
}

void Logmanager::printSameRooms(vector<pair<string, bool> > allUsers, bool toHtml){
	HTML htmlprint;
	if(toHtml){
		htmlprint.header();
		htmlprint.startTable();
		htmlprint.addHeaderToTable("Rooms");
	}
	map<int,pair<int,int> > timeWindows;
	map<int,bool> visited;
	bool first = true;
	for(vector<pair<string,bool> >::iterator iter = allUsers.begin(); iter!=allUsers.end(); iter++){
		map<string,vector<visit> >::iterator it2 = artlog.find(iter->first);
		if(it2 == artlog.end() || it2->second.back().employer != iter->second){
			if(toHtml){
				htmlprint.endTable();
				htmlprint.footer();
			}
			return;
		}
		int indicatorThatWasThereBefore = 0;
		for(vector<visit>::iterator rit = it2->second.begin(); rit!=it2->second.end(); rit++){
			int room = rit->room;
			bool arrival = rit->arrival;
			int timestamp = rit->timeStamp;
			if(first == true){
				if(arrival && room != -1){
					pair<int,int> tmp;
					tmp.first = timestamp;
					tmp.second = -1;
					timeWindows[room] = tmp;
					visited[room] = false;
				}else if(arrival == false && room != -1){
					timeWindows[room].second = timestamp;
				}
			}else{
				map<int,pair<int,int> >::iterator found = timeWindows.end();
				if(room != -1){
					found = timeWindows.find(room);
					if(found == timeWindows.end()){
						continue;
					}
				}else{
					continue;
				}
				if(arrival){
					if(found->second.first < timestamp && (found->second.second > timestamp || found->second.second == -1)){
						visited[room] = true;
						found->second.first = timestamp;
					}else if(found->second.first > timestamp){
						indicatorThatWasThereBefore = 1;
						visited[room] = true;
					}
				}else if(arrival == false){
					if(found->second.first < timestamp && (found->second.second > timestamp || found->second.second == -1)){
						visited[room] = true;
						found->second.second = timestamp;
					}else if(found->second.second < timestamp && indicatorThatWasThereBefore == 1){
						indicatorThatWasThereBefore = 0;
						visited[room] = true;
					}else if(indicatorThatWasThereBefore == 1){
						visited[room] = false;
						indicatorThatWasThereBefore = 0;
					}
				}
			}
		}
		if(!first){
			for(map<int,bool>::iterator booliter = visited.begin(); booliter!=visited.end(); booliter++){
				if(booliter->second == false){
					timeWindows.erase(booliter->first);
				}
			}
			visited.clear();
			for(map<int,pair<int,int> >::iterator pairiter = timeWindows.begin(); pairiter!=timeWindows.end(); pairiter++){
				visited[pairiter->first] = false;
			}

		}
		first = false;
	}
	first = true;
	for(map<int,pair<int,int> >::iterator pairiter = timeWindows.begin(); pairiter!=timeWindows.end(); pairiter++){
		if(toHtml){
			htmlprint.addElementToTable(to_string(pairiter->first));
		}else{
			if(first){
				cout << pairiter->first;
				first = false;
			}else{
				cout << "," << pairiter->first;
			}
		}
	}
	cout << endl;

	if(toHtml){
		htmlprint.endTable();
		htmlprint.footer();
	}

}