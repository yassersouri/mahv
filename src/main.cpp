#include <iostream>
#include <string>
#include <fstream>
#include <vector>
#include "json11.hpp"
#include "ncursesHelper.cpp"

using namespace std;

int main(int argc, char **argv) {
	vector<string> *mainChoices = new vector<string>();
	mainChoices->push_back("Benchmark");
	mainChoices->push_back("Exit");

	string welcomString = "Use arrays to select\nSelect What action you like to make";
	string statusString = "You choose number %d with choice name: %s\n";

	const char *ws = welcomString.c_str();
	const char *ss = statusString.c_str();

	string fileAddress = "/Users/yasser/sharif-repo/mahv/benchmark-dataset/info.json";
	string fileContent;
	string err;

	ifstream ifs(fileAddress);
	fileContent.assign( (istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));

	json11::Json benchmarks = json11::Json::parse(fileContent, err);

	if (argc > 1) {
		cout << "going to be implemented" << endl;
	} else {
		int choice = runCurses(ws, ss, mainChoices);
		if (choice == 1) {
			string bw = "Use arrays to select\nSelect What benchmark to run";
			const char *bwcs = bw.c_str();
			vector<string> *benchChoices = new vector<string>();

			for(vector<string>::size_type i = 0; i != benchmarks["benchmarks"].array_items().size(); ++i) {
				benchChoices->push_back(benchmarks["benchmarks"].array_items().at(i).string_value());
			}

			int choice = runCurses(bwcs, ss, benchChoices);
			cout << choice << endl;
		} else {
			cout << "Exit Selected!" << endl;
			return 0;
		}
	}
	return 0;
}

