#include "DataParse.h"

DataParse::DataParse() 
{

}
DataParse::DataParse(string url) 
{
	ReadData(url);
}
void DataParse::ReadData(string url) 
{
	std::ifstream ifs(url);
	if (!ifs.is_open())
	{
		//std::cout << "Could not open file for reading!\n";
		return;
	}
	IStreamWrapper isw{ ifs };

	Doc.ParseStream(isw);

	StringBuffer buffer{};
	Writer<StringBuffer> writer{ buffer };
	Doc.Accept(writer);
	return;
}
vector<pair<string, string>> DataParse::GetOrderData() 
{
	vector<pair<string,string>> v;
	auto data = Doc.GetObj();
	for (auto i = data.begin(); i != data.end(); i++) 
	{
		v.push_back(make_pair(i->name.GetString(),i->value["index"].GetString()));
	}
	return v;
}
string DataParse::GetData(string idx,string src) 
{
	if (Doc.HasMember(idx.c_str())) 
	{
		if (Doc[idx.c_str()].HasMember(src.c_str())) 
		{
			return Doc[idx.c_str()][src.c_str()].GetString();
		}
	}
	return "-1";
}

string DataParse::AssignData(string str,vector<string> v) 
{
	vector<string> src= Manager::getIncetance().Split(str, "%");
	int i = 0;
	string data="";
	for (string s : src) 
	{
		if (s == "/d") 
		{
			s = v[i++];
		}
		data += s;
	}
	return data;
}
