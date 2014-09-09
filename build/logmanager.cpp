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
	file.seekg(0, ios::end);
	if(file.tellg() == 0){
		file.close();
		return;
	}
	file.seekg(0,ios::beg);
	string holeFile((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	int fileLength = holeFile.length();	

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
	if((arrival == true && iter == artlog.end() && room == -1) ||
	   (arrival == true && iter != artlog.end() && room != -1 && ((iter->second[0].arrival == false && iter->second[0].room != -1) || (iter->second[0].arrival == true && iter->second[0].room == -1))) ||
	   (arrival == false && iter != artlog.end() && room == -1 && ((iter->second[0].arrival == false && iter->second[0].room !=-1) || (iter->second[0].arrival == true && iter->second[0].room == -1))) ||
	   (arrival == false && iter != artlog.end() && room != -1 && ((iter->second[0].arrival == true && iter->second[0].room == room)))){
	   	if(iter != artlog.end()){
	   		newvis.lowerTime = iter->second[0].lowerTime;
	   	}
		artlog[name].insert(artlog[name].begin(),newvis);
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
		for(int i=0; i<iter->second.size(); i++){
			cout << "\tEvent " << i << ") timestamp = " << iter->second[i].timeStamp << " employer = "
				<< iter->second[i].employer << " room = " << iter->second[i].room << " arrival = "
				<< iter->second[i].arrival << " [" << iter->second[i].lowerTime << "," << iter->second[0].upperTime << "]" << endl;
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
		if(iter->second.front().arrival==false && iter->second.front().room==-1){
			continue;
		}else{
			if(iter->second.front().employer){
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
		if(artlog[temp].front().arrival==false){
			//They are not in a room
			continue;
		}else{
			int room = artlog[temp].front().room;
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

void Logmanager::printUserData(string user, bool toHtml){
	HTML htmlprint;
	if(toHtml){
		htmlprint.header();
		htmlprint.startTable();
		htmlprint.addHeaderToTable("Rooms");
	}
	map<string,vector<visit> >::iterator it = artlog.find(user);
	if(it == artlog.end()){
		return;
	}
	vector<visit> allVisits = it->second;
	vector<int> roomsEntered;
	for(unsigned int i=0; i<allVisits.size(); i++){
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
		if(iter->second[0].employer == false){
			continue;
		}
		visit temp = iter->second[0];
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
void Logmanager::totalTimeOfUser(string user){
	map<string,vector<visit> >::iterator it = artlog.find(user);
	if(it == artlog.end()){
		return;
	}

	if(it->second[0].upperTime == -1){
		cout << currMaxTime - it->second[0].lowerTime << endl;
	}else{
		cout << it->second[0].upperTime - it->second[0].lowerTime << endl;
	}
}