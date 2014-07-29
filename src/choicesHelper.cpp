#include <string>
#include <fstream>
#include <vector>
#include "json11.hpp"
#include <opencv2/opencv.hpp>
#include "cvHelper.cpp"
#include <iostream>

using namespace std;

string BASE_BENCH_ADR = "/Users/yasser/sharif-repo/mahv/benchmark-dataset/";
string ORIG_APPEND = "-0-original.jpg";
string MASK_APPEND = "-1-masked.jpg";

vector<string> *getBenchNames() {
	string fileAddress = BASE_BENCH_ADR + "info.json";
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

vector<string> *getMainChoices() {
	vector<string> *mainChoices = new vector<string>();
	mainChoices->push_back("Benchmark");
	mainChoices->push_back("Exit");

	return mainChoices;
}

void getOrigMask(string benchName, cv::Mat &orig, cv::Mat &mask) {
	orig = cv::imread(BASE_BENCH_ADR + benchName + ORIG_APPEND);
	cv::Mat maskImg = cv::imread(BASE_BENCH_ADR + benchName + MASK_APPEND);
	extractMaskFromBench(maskImg, mask);
}
