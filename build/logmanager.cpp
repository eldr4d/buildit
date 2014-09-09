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
#include "printHTML.hpp"

using namespace std;

bool compare( const string &left, const string &right ) {
	for( string::const_iterator lit = left.begin(), rit = right.begin(); lit != left.end()-1 && rit != right.end()-1; lit++, rit++ ){
		if( ( *lit ) < ( *rit ) ){
			return true;
		}
		else if( ( *lit ) > ( *rit ) ){
			return false;
		}
	}
	if( left.size() < right.size() ){
		return true;
	}
	return false;
}

bool compare2( const string &left, const string &right ) {
	for( string::const_iterator lit = left.begin(), rit = right.begin(); lit != left.end() && rit != right.end(); lit++, rit++ ){
		if( ( *lit ) < ( *rit ) ){
			return true;
		}
		else if( ( *lit ) > ( *rit ) ){
			return false;
		}
	}
	if( left.size() < right.size() ){
		return true;
	}
	return false;
}



void Logmanager::serialize(){
 	ofstream file(logFileName.c_str(), ios::out | ios::binary);
 	stringstream ss;
	boost::archive::text_oarchive oarch(ss);
	oarch << artlog;
	string holeFile = ss.str();

	int fileLength = holeFile.length();
	while(fileLength%8 != 0){
		fileLength++;
		holeFile.append("-");
	}
	//cout << fileLength << endl;
	//cout << holeFile << endl;
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
	//cout << unencryptedData << endl;
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
	map<string, vector<visit> >::iterator iter = artlog.find(name);
	if((arrival == true && iter == artlog.end() && room == -1) ||
	   (arrival == true && iter != artlog.end() && room != -1 && ((iter->second[0].arrival == false && iter->second[0].room != -1) || (iter->second[0].arrival == true && iter->second[0].room == -1))) ||
	   (arrival == false && iter != artlog.end() && room == -1 && ((iter->second[0].arrival == false && iter->second[0].room !=-1) || (iter->second[0].arrival == true && iter->second[0].room == -1))) ||
	   (arrival == false && iter != artlog.end() && room != -1 && ((iter->second[0].arrival == true && iter->second[0].room == room)))){
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
		tmp.erase(tmp.length()-1,1);
		cout << "User = " << tmp << endl;
		for(int i=0; i<iter->second.size(); i++){
			cout << "\tEvent " << i << ") timestamp = " << iter->second[i].timeStamp << " employer = " << iter->second[i].employer << " room = " << iter->second[i].room << " arrival = " << iter->second[i].arrival << endl;
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
	sort(employersInGallery.begin(), employersInGallery.end(), compare);
	sort(guestsInGallery.begin(), guestsInGallery.end(), compare);

	bool first;
	if(toHtml){
		htmlprint.addDoubleHeaderToTable("Employee", "Guest");
		int big =  employersInGallery.size() > guestsInGallery.size() ?  employersInGallery.size() : guestsInGallery.size();
		for(unsigned int i=0; i<big; i++){
			string data1, data2;
			if(i < employersInGallery.size()){
				data1 = employersInGallery[i];
				data1.erase(data1.length()-1,1);
			}else{
				data1 = "";
			}
			if(i < guestsInGallery.size()){
				data2 = guestsInGallery[i];
				data2.erase(data2.length()-1,1);
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
	}

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
			if(room == -1){
				continue;
			}
			temp.erase(temp.length()-1,1);
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
		sort(iter->second.begin(), iter->second.end(), compare2);
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