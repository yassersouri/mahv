#include <string>
#include <fstream>
#include <vector>
#include "json11.hpp"

using namespace std;

vector<string> *getBenchNames() {
	string fileAddress = "/Users/yasser/sharif-repo/mahv/benchmark-dataset/info.json";
	string fileContent;
	string err;

	ifstream ifs(fileAddress);
	fileContent.assign( (istreambuf_iterator<char>(ifs)), (istreambuf_iterator<char>()));

	json11::Json benchmarks = json11::Json::parse(fileContent, err);

	vector<string> *benchChoices = new vector<string>();
	for(vector<string>::size_type i = 0; i != benchmarks["benchmarks"].array_items().size(); ++i) {
		benchChoices->push_back(benchmarks["benchmarks"].array_items().at(i).string_value());
	}

	return benchChoices;
}

vector<string> getMainChoices() {
	vector<string> *mainChoices = new vector<string>();
	mainChoices->push_back("Benchmark");
	mainChoices->push_back("Exit");

	return mainChoices;
}
