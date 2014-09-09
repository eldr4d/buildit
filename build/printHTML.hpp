#include <iostream>
#include <string>


class HTML{
public:
	HTML(){};
	void header(){
		std::cout << "<html>";// << std::endl;
		std::cout << "<body>";// << std::endl;
	}

	void footer(){
		std::cout << "</body>";// << std::endl;
		std::cout << "</html>";// << std::endl;
	}

	void startTable(){
		std::cout << "<table>";// << std::endl;
	}

	void endTable(){
		std::cout << "</table>";// << std::endl;
	}

	void addElementToTable(std::string data){
		std::cout << "<tr>";// << std::endl;
		std::cout << "<td>" << data << "</td>";// << std::endl;
		std::cout << "</tr>";// << std::endl;
	}

	void addHeaderToTable(std::string data){
		std::cout << "<tr>";// << std::endl;
		std::cout << "<th>" << data << "</th>";// << std::endl;
		std::cout << "</tr>";// << std::endl;
	}

	void addDoubleElementToTable(std::string data1, std::string data2){
		std::cout << "<tr>";// << std::endl;
		std::cout << "<td>" << data1 << "</td>";// << std::endl;
		std::cout << "<td>" << data2 << "</td>";// << std::endl;
		std::cout << "</tr>";// << std::endl;
	}

	void addDoubleHeaderToTable(std::string data1, std::string data2){
		std::cout << "<tr>";// << std::endl;
		std::cout << "<th>" << data1 << "</th>";// << std::endl;
		std::cout << "<th>" << data2 << "</th>";// << std::endl;
		std::cout << "</tr>";// << std::endl;
	}
};