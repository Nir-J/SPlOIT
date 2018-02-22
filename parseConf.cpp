#include <iostream>
#include <sstream>
#include <string>
#include <unordered_map>
#include <algorithm>   // need to see if this is actually required
#include <fstream>
#include <vector>

#define CONFIG_FILE "sploit.conf"

typedef std::unordered_map<std::string, std::pair<std::string, int> > UserMap;
typedef std::unordered_map<std::string, std::pair<std::vector<std::string>, int> > CmdMap;
unsigned int PORT = -1;
std::string base = "";

int main() {

	using namespace std;
	UserMap users;
	CmdMap commands;

	ifstream configFile(CONFIG_FILE);

	if(configFile.is_open()) {

		cout << "Hurray I can open the file" << endl;
		string line;
		while(getline(configFile, line)) {
			//line.erase(std::remove_if(line.begin(), line.end(), std::isspace), line.end());
		//	line.erase(remove_if(line.begin(), line.end(), [] (char c){ return std::isspace(static_cast<unsigned char>(c));}));
			// ltrim only
			line.erase(line.begin(), find_if(line.begin(), line.end(), [](int ch) {return !std::isspace(ch);}));

			if(line[0] == '#' || line.empty()) {
				continue;
			}
		
			//vector<string> tokens;
			istringstream iss(line);
			vector<string> tokens(istream_iterator<string>{iss}, istream_iterator<string>());

			for(auto token = tokens.begin(); token != tokens.end(); token++) {
				cout << "Token is " << *token << endl;

				if((*token).compare("base") == 0) {
					base = *++token;
					break;
				} else if((*token).compare("user") == 0) {
					string user = *++token;
					string pass = *++token;
					users.insert(make_pair(user, make_pair(pass, 0)));
					break;
				} else if((*token).compare("port") == 0) {
					PORT = stoi(*++token);
					break;
				} else if((*token).compare("alias") == 0) {
				
					string alias = *++token;
					string cmd = *++token;
					
					vector<string> params(token, tokens.end());
					commands.insert(make_pair(alias, make_pair(params, 0)));
					break;
				} else {
					cout << "I have no idea about this config option" << endl;
				}

						
			}

		}

		configFile.close();	
			

		cout << "---------CONFIG READ--------------" << endl;
		cout << "base is " << base << endl;
		cout << "port is " << PORT << endl;
		cout << "Users are : " << endl;
		for(auto it = users.begin(); it != users.end(); it++) {
			cout << (*it).first << "\t" << (*it).second.first << endl;
		}
		
		for(auto it = commands.begin(); it != commands.end(); it++) {
			cout << (*it).first << "\t";
			for(auto jt = (*it).second.first.begin(); jt != (*it).second.first.end(); jt++) {
				cout << (*jt) << " ";
			}
			cout << endl;
		}
		cout << "------------END-------------------" << endl;
	} else {
		std::cout << "Unable to open config file. " << std::endl;
	}

	return 0;
} 





