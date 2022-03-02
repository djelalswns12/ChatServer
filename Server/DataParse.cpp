#include "DataParse.h"

DataParse::DataParse() {

}
DataParse::DataParse(string url) {
	ReadData(url);
}
void DataParse::ReadData(string url) 
{
	std::ifstream ifs(url);
	if (!ifs.is_open())
	{
		std::cout << "Could not open file for reading!\n";
		return;
	}
	IStreamWrapper isw{ ifs };

	doc.ParseStream(isw);

	StringBuffer buffer{};
	Writer<StringBuffer> writer{ buffer };
	doc.Accept(writer);
	//cout << doc.GetObject().HasMember("2H");
	//cout << doc.GetObject().begin()->value["index"].GetString();
	return;
}
vector<pair<string, string>> DataParse::GetOrderData() 
{
	vector<pair<string,string>> v;
	auto data = doc.GetObj();
	for (auto i = data.begin(); i != data.end(); i++) {
		v.push_back(make_pair(i->name.GetString(),i->value["index"].GetString()));
	}
	return v;
}
string DataParse::GetData(string idx,string src) 
{
	if (doc.HasMember(idx.c_str())) {
		if (doc[idx.c_str()].HasMember(src.c_str())) {
			return doc[idx.c_str()][src.c_str()].GetString();
		}
	}
	return "-1";
}
vector<string> DataParse::split(string ids, string target) {
	vector<string> names;
	size_t cur, pre = 0;
	cur = ids.find(target);
	while (cur != string::npos) {
		string substring = ids.substr(pre, cur - pre);
		if (substring.length() > 0) {
			names.push_back(substring);
		}
		pre = cur + target.length();
		cur = ids.find(target, pre);
	}
	if (ids.substr(pre, cur - pre).length() > 0) {
		names.push_back(ids.substr(pre, cur - pre));
	}
	return names;
}
string DataParse::AssignData(string str,vector<string> v) {
	vector<string> src=split(str, "%");
	int i = 0;
	string data="";
	for (string s : src) {
		cout << s;
		if (s == "/d") {
			s = v[i++];
		}
		data += s;
	}
	return data;
}