#include <sstream>
#include <fstream> 
#include <algorithm>

#include <boost/serialization/serialization.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>

#include "logmanager.hpp"
#include "blowfish.hpp"

using namespace std;

void Logmanager::serialize(){
 	ofstream file(logFileName.c_str(), ios::in | ios::binary);
 	stringstream ss;
	boost::archive::text_oarchive oarch(ss);
	oarch << artlog;
	string holeFile = ss.str();

	int fileLength = holeFile.length();
	while(fileLength%8 != 0){
		fileLength++;
		holeFile.push_back('-');
	}
	cout << holeFile << endl;
	cout << "ser " << fileLength << endl;
	unsigned char *encryptedData1, *encryptedData2;
	encryptedData1 = new unsigned char[fileLength];
	encryptedData2 = new unsigned char[fileLength];

	CBlowFish oBlowFish((unsigned char*)secret.c_str(), secret.length());
	oBlowFish.Encrypt((unsigned char*)holeFile.c_str(), encryptedData1, fileLength, CBlowFish::CBC);
	//oBlowFish.Encrypt(encryptedData1, encryptedData2, fileLength, CBlowFish::CBC);
	//cout << encryptedData1 << endl;
	//file << holeFile;
	file.write((const char*)encryptedData1,fileLength);
	file.close();
}

void Logmanager::deserialize(){
	ifstream file(logFileName.c_str(), ios::out | ios::binary);
	file.seekg(0, ios::end);
	if(file.tellg() == 0){
		file.close();
		return;
	}
	file.seekg(0,ios::beg);
	string holeFile((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
	int fileLength = holeFile.length();	

	unsigned char *unencryptedData;
	unencryptedData = new unsigned char[fileLength];
	cout << "des" << fileLength << endl;
	CBlowFish oBlowFish((unsigned char*)secret.c_str(), secret.length());
	oBlowFish.Decrypt((unsigned char*)holeFile.c_str(), unencryptedData, fileLength, CBlowFish::CBC);
	//oBlowFish.Encrypt(encryptedData1, encryptedData2, fileLength, CBlowFish::CBC);
	//cout << unencryptedData << endl;
	while(unencryptedData[fileLength-1] == '-'){
		unencryptedData[fileLength-1] = '\0';
		fileLength--;
	}
	stringstream ss;
	ss << unencryptedData;
	boost::archive::text_iarchive iarch(ss); 
	iarch >> artlog;
	file.close();
}

int Logmanager::append(string name, int timestamp, bool employer, int room, bool arrival){
	//Wrong timestamp
	if(currMaxTime > timestamp){
		return -1;
	}
	currMaxTime = timestamp;
	visit newvis;
	newvis.employer = employer;
	newvis.arrival = arrival;
	newvis.room = room;
	newvis.timeStamp = timestamp;
	map<string, vector<visit> >::iterator iter = artlog.find(name);

	//Wrong first arrival 
	if(iter == artlog.end() && (newvis.arrival == false || newvis.room != -1)){
		return -1;
	}
	//Wrong leave
	///if(iter != artlog.end() && iter->second[0].arrival  < )


	artlog[name].insert(artlog[name].begin(),newvis);
}

void Logmanager::prettyPrint(){
	for(map<string, vector<visit> >::iterator iter = artlog.begin(); iter != artlog.end(); iter++){
		string tmp = iter->first;
		tmp.erase(tmp.length()-1,1);
		cout << "User = " << tmp << endl;
		for(int i=0; i<iter->second.size(); i++){
			cout << "\tEvent " << i << ") timestamp = " << iter->second[i].timeStamp << " employer = " << iter->second[i].employer << " room = " << iter->second[i].room << " arrival = " << iter->second[i].arrival << endl;
		}
	}
}


void Logmanager::printState(){
	vector<string> employersInGallery;
	vector<string> guestsInGallery;
	for(map<string, vector<visit> >::iterator iter = artlog.begin(); iter != artlog.end(); iter++){
		if(iter->second.front().arrival==false && iter->second.front().room==-1){
			continue;
		}else{
			if(iter->second.front().employer){
				employersInGallery.push_back(iter->first);
			}else{
				guestsInGallery.push_back(iter->first);
			}
		}
	}

	bool first = true;
	for(unsigned int i=0; i<employersInGallery.size(); i++){
		string tmp = employersInGallery[i];
		tmp.erase(tmp.length()-1,1);
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
		tmp.erase(tmp.length()-1,1);
		if(first)
			cout << tmp;
		else
			cout << "," << tmp;
		first = false;
	}
	cout << endl;

	map<int,vector<string> > rooms;
	vector<string> totalPersons;
	totalPersons.reserve(employersInGallery.size() + guestsInGallery.size());
	totalPersons.insert(totalPersons.end(),employersInGallery.begin(),employersInGallery.end());
	totalPersons.insert(totalPersons.end(),guestsInGallery.begin(),guestsInGallery.end());

	for(unsigned int i=0; i< totalPersons.size(); i++){
		string temp = totalPersons[i];
		if(artlog[temp].front().arrival==false){
			//They are not in a room
			continue;
		}else{
			int room = artlog[temp].front().room;
			temp.erase(temp.length()-1,1);
			rooms[room].push_back(temp);
		}
	}

	for(map<int,vector<string> >::iterator iter = rooms.begin(); iter!=rooms.end(); iter++){
		cout << iter->first << ":";
		sort(iter->second.begin(), iter->second.end());
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

void Logmanager::printUserData(string user){
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
			cout << roomsEntered[i];
		}else{
			cout << "," << roomsEntered[i];
		}
		first = false;
	}
	cout << endl;
}