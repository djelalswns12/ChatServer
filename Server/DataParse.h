#pragma once
#include <fstream>
#include <cstring>
#include <vector>
#include <iostream>
#include <cstdlib>
#include "rapidjson/document.h"
#include "rapidjson/istreamwrapper.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/ostreamwrapper.h"
using namespace rapidjson;
using namespace std;
class DataParse
{
public:
	Document doc;
	DataParse();
	DataParse(string);
	void ReadData(string);
	vector<pair<string, string>> GetOrderData();
	string GetData(string,string);
	string AssignData(string,vector<string>);
	vector<string> split(string, string);
};
