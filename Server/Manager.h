#pragma once
#pragma comment( lib, "ws2_32.lib")
#pragma warning(disable:4996)
#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <tchar.h>
#include <locale.h>
#include <WS2tcpip.h>
#include <map>
#include <set>
#include <vector>
#include <string>
#include <ctime>
#include "DataParse.h"
#include "USER.h"
#include "ROOM.h"


using namespace std;

const int PACKET_SIZE{ 1024 };

enum class EState;
class DataParse;
class USER;
class ROOM;

typedef void (Manager::* OrderFunc)(USER*, vector<string>&);
class Manager
{
private:
	Manager() { }
	Manager(const Manager& ref) {}
	Manager& operator=(const Manager& ref) {}
	~Manager() {}
public:
	static Manager& getIncetance() 
	{
		static Manager ins;
		return ins;
	}

	SOCKET ServerSocket;
	WSADATA SocketData;//현재 이 소켓프로그래밍에서 사용할 소켓정보
	SOCKADDR_IN SocketAddr; // 서버 IP , PORT 구조체
	short Port = 4444;

	map<SOCKET, USER*> UserList;
	map<string, USER*> NameList;
	vector<ROOM> RoomList;

	DataParse* DB;

	map<string, OrderFunc> OrderFuncs;
	vector<OrderFunc> OrderFuncsList{ &Manager::H,&Manager::H_,&Manager::US,&Manager::LT,&Manager::J,&Manager::O,&Manager::TO,&Manager::ST,&Manager::PF,&Manager::Login,&Manager::Q,&Manager::IN_,&Manager::X };

	string GetNowTime();
	vector<string> Split(const string&, string);
	vector<string> Split(const string&, string, int);
	bool IsNumber(string);
	void ServerON(string);
	void InitSocket();
	void CreateSocket();
	void SetSocketInfo();
	void Bind();
	void Listen();

	void DisConnectRoom(SOCKET*);
	void DisConnect(SOCKET*);
	int FindEmptyRoomIdx();
	void Print(const string&);

	void SendPrompt(USER*);
	void Msg(USER*,string&);
	void Login(USER*, vector<string>&);
	void H(USER*, vector<string>&);
	void H_(USER*, vector<string>&);
	void US(USER*, vector<string>&);
	void LT(USER*, vector<string>&);
	void J(USER*, vector<string>&);
	void O(USER*, vector<string>&);
	void TO(USER*, vector<string>&);
	void ST(USER*, vector<string>&);
	void PF(USER*, vector<string>&);
	void Q(USER*, vector<string>&);
	void X(USER*, vector<string>&);
	void IN_(USER*, vector<string>&);

	void InsertUser(USER*);
	void RemoveUser(SOCKET*);
	
	void UserChangeEvent();
	void RoomChangeEvent();

	void SetOrder(vector<pair<string, string>>);
	bool ExcuteOrder(USER*, vector<string>&);
	//void X(USER*, vector<string>&);
};

