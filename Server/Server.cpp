#pragma comment( lib, "ws2_32.lib")

#define PACKET_SIZE 1024
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

using namespace std;

enum class State
{
	auth,
	lobby,
	room
};

//////////////////////////////////////////////////////////////
//////////				   클래스				//////////////
//////////////////////////////////////////////////////////////
class USER;
class ROOM;

class USER {
private:
	string name;
	string joinTime;
	State state;


public:
	SOCKET socket;
	vector<char> buffer;
	SOCKADDR_IN addr;
	
	USER() {
		
	}
	USER(SOCKET socket, SOCKADDR_IN addr) {
		this->socket = socket;
		this->state = State::auth;
		this->addr = addr;
		name = "";
	}
	void SetName(string s) {
		name = s;
	}
	string GetName() {
		return name;
	}
	void SetState(State st) {
		this->state = st;
	}
	char* GetIP() {
		return inet_ntoa(addr.sin_addr);
	}
	short GetPort() {
		return ntohs(addr.sin_port);
	}
	State GetState() {
		return this->state;
	}
	bool operator < (const USER& ref) const {
		return name<ref.name;
	}

};

class ROOM {
private:
	bool isOpen;
	string name;
	string owner;
	string openTime;
	int maxCnt;
	int curCnt;
	set<USER> users;
public:
	ROOM() {
		SetOpen(false);
	}

	void SetROOM(string name, string owner, string openTime, int maxCnt) {
		this->name = name;
		this->owner = owner;
		this->openTime = openTime;
		this->maxCnt = maxCnt;
		SetOpen(true);
	}
	void SetOpen(bool state) {
		isOpen = state;
	}
	bool GetOpen() {
		return isOpen;
	}
	void SetUser(USER* user) {
		users.insert(*user);
		user->SetState(State::room);
	}
	set<USER> GetUsers() {
		return users;
	}
	string GetName() {
		return name;
	}
};
//////////////////////////////////////////////////////////////
//////////					변수				//////////////
//////////////////////////////////////////////////////////////
SOCKET ServerSocket;

//유저 관련
map<SOCKET, USER*> UserList;
map<string, USER*> NameList;

//룸 관련
vector<ROOM> RoomList(100);



//////////////////////////////////////////////////////////////
//////////				문자열 관련	함수		//////////////
//////////////////////////////////////////////////////////////
vector<string> split(string ids, string target) {
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
bool isNumber(string s) {
	for (char c : s) {
		if (isdigit(c) == 0) {
			return false;
		}
	}
	return true;
}

//////////////////////////////////////////////////////////////
//////////				서버 관련 함수			//////////////
//////////////////////////////////////////////////////////////
char* AssembleBuffer(vector<char>* str) {
	char* out;
	char data[PACKET_SIZE];
	int i;
	for (i = 0; i < str->size() - 1; i++) {
		data[i] = *(str->begin() + i);
	}
	//명령어 확인
	out = new char[i + 1];
	out[i] = '\0';
	memcpy(out, data, i);
	cout << out << "\n";
	return out;
}

void SendMsg(SOCKET& socket, const char c[]) {
	send(socket, c, strlen(c), 0);
}


void OnDisConnect(SOCKET* tmp) {
	NameList.erase(UserList.at(*tmp)->GetName());
	UserList.erase(*tmp);
	closesocket(*tmp);
	cout << UserList.size() << "\n";

}
void Login(USER *user, string name) {
	/*
	닉네임 설정 후
	닉네임 리스트에 추가
	유저 상태 변경
	*/
	if (user->GetState() != State::auth) {
		//이미 로그인됨
		cout << "이미 로그인됨";
		return;
	}
	if (NameList.find(name) == NameList.end()) {
		user->SetName(name);
		NameList.insert(make_pair(name,user ));
		user->SetState(State::lobby);
		SendMsg(user->socket, "-----------------------------------------------------\r\n 반갑습니다. 텍스트 채팅 서버 ver 0.1 입니다.\r\n 이용중 불편하신 점이 있으면 아래 이메일로 문의 바랍니다.\r\n 감사합니다.\r\n\t\tprogrammed & arranged by MIN JOON \r\n\t\temail:djelalswns12@naver.com\r\n-----------------------------------------------------\r\n");
		SendMsg(user->socket, "명령어안내(H) 종료(X)\r\n");
		SendMsg(user->socket, "선택>");
	}
	else {
		SendMsg(user->socket, "**아이디를 이미 사용중입니다.다른 아이디를 사용해주세요.\r\n");
	}
}
int FindEmptyRoomIdx() {
	for (int i = 0; i < RoomList.size(); i++) 
	{
		if (RoomList[i].GetOpen() == false) 
		{
			return i;
		}
	}
	return -1;
}


int main()
{

	WSADATA soData;//현재 이 소켓프로그래밍에서 사용할 소켓정보
	SOCKADDR_IN soAddr; // 서버 IP , PORT 구조체

	short _port = 4444;

	// MAKEWORD = 사용할 소켓버전 선택 , socekt함수에서 사용될 소켓 버전설정
	if (WSAStartup(MAKEWORD(2, 2), &soData) != 0) {
		cout << "소켓 정보 로드 실패";
	}

	//PF_INET = IPV4
	//SOCK_STREAM = TCP / IP
	ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	if (ServerSocket == INVALID_SOCKET) {
		cout << "소켓 생성 실패\n";
	}

	//_serverAddr세팅
	memset(&soAddr, 0, sizeof(soAddr));
	soAddr.sin_family = AF_INET;
	soAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //IP설정
	soAddr.sin_port = htons(_port);

	if (bind(ServerSocket, (SOCKADDR*)&soAddr, sizeof(soAddr)) == SOCKET_ERROR) {
		cout << "소켓 바인드 실패\n";
	}

	if (listen(ServerSocket, 5) == SOCKET_ERROR) {
		cout << "소켓 리슨 실패\n";
	}
	cout << "Server ON\n\n";

	fd_set read, tmp;
	TIMEVAL time;
	FD_ZERO(&read);
	FD_SET(ServerSocket, &read);
	int re;
	while (true)
	{
		tmp = read;
		time.tv_sec = 1;  // 초기화가 매번 이루어져야 한다.
		time.tv_usec = 0;
		re = select(NULL, &tmp, NULL, NULL, &time);
		if (SOCKET_ERROR == re)break;
		if (0 == re)continue;

		for (int i = 0; i < read.fd_count; i++)
		{
			if (FD_ISSET(read.fd_array[i], &tmp))
			{
				if (ServerSocket == read.fd_array[i])
				{
					SOCKADDR_IN caddr;
					int size = sizeof(caddr);
					SOCKET cid = accept(ServerSocket, (SOCKADDR*)&caddr, &size);
					USER *user = new USER(cid, caddr);
					UserList.insert(make_pair(cid, user));
					FD_SET(cid, &read);
					SendMsg(cid, "채팅에 온걸 환영합니다.\r\n\0");
					printf("%s:%d connected\n", UserList.at(cid)->GetIP(), UserList.at(cid)->GetPort());
				}
				else
				{
					char c;
					int len = recv(read.fd_array[i], &c, sizeof(char), 0);
					if (0 == len)
					{
						printf("[%d] LogOut\n", read.fd_array[i]);
						FD_CLR(read.fd_array[i], &read);
						OnDisConnect(&tmp.fd_array[i]);
						//closesocket(tmp.fd_array[i]);
					}
					else
					{
						if (UserList.at(read.fd_array[i])->buffer.size() > 0 && *(UserList.at(read.fd_array[i])->buffer.end() - 1) == '\r' && c == '\n')
						{
							cout << UserList[read.fd_array[i]]->GetIP() << ":" << UserList[read.fd_array[i]]->GetPort() << " [" << UserList[read.fd_array[i]]->GetName() << "]" << ":";
							char* msg = AssembleBuffer(&UserList[read.fd_array[i]]->buffer);
							//LOGIN
							string msgString(msg);
							vector<string> orderList = split(msgString, " ");

							switch (UserList[read.fd_array[i]]->GetState())
							{
							case State::auth:
								if (orderList.size() > 0)
								{
									if (orderList[0] == "LOGIN")
									{
										if (orderList.size() > 1 && orderList[1].length() > 0) {
											SendMsg(read.fd_array[i], "**로그인 시도.\r\n");
											Login(UserList[read.fd_array[i]], orderList[1]);
										}
										else {
											SendMsg(UserList[read.fd_array[i]]->socket, "** 올바른 사용법은 LOGIN [ID] 입니다.\r\n");
										}
									}
									else {
										cout << UserList[read.fd_array[i]]->GetPort()<<"\n";
										SendMsg(UserList[read.fd_array[i]]->socket, "**로그인 명령어(LOGIN)를 사용해주세요.\r\n");
									}
								}
								break;
							case State::lobby:
								if (orderList.size() > 0)
								{
									if (orderList[0] == "H")
									{
										SendMsg(UserList[read.fd_array[i]]->socket, "-----------------------------------------------------\r\nH\t\t\t명령어 안내\r\nUS\t\t\t이용자 목록 보기\r\nLT\t\t\t대화방 목록 보기\r\nST [방번호]\t\t대화방 정보 보기\r\nPF [상대방ID]\t\t이용자 정보 보기\r\nTO [상대방ID] [메시지]\t쪽지 보내기\r\nO  [최대인원] [방제목]\t대화방 만들기\r\nJ  [방번호]\t\t대화방 참여하기\r\nX\t\t\t끝내기\r\n-----------------------------------------------------\r\n");
									}
									else if (orderList[0] == "US")
									{
									
									}
									else if (orderList[0] == "LT")
									{
										for (auto iter = RoomList.begin();iter!=RoomList.end();iter++) {
											if (iter->GetOpen() == true) {
												SendMsg(UserList[read.fd_array[i]]->socket, (iter->GetName()+"\r\n").c_str());
											}
										}
									
									}
									else if (orderList[0] == "ST")
									{
										
									}
									else if (orderList[0] == "PF")
									{
									
									}
									else if (orderList[0] == "TO")
									{
									
									}
									else if (orderList[0] == "O")
									{
										//대화방 만들기

										/*
										** 올바른 사용법은 O [최대인원] [방제목] 입니다.
										** 대화방 인원을 2-20명 사이로 입력해주세요.
										** 대화방 제목이 필요합니다.
										** 대화방이 개설되었습니다.
										** ' '님이 들어오셨습니다. (현재인원 1/3)
										*/
										if (orderList.size() > 1)
										{
											if (isNumber(orderList[1]) == true && stoi(orderList[1]) >= 2) {
												if (orderList.size() > 2 && orderList[2].length() > 2) {
													ROOM* room= &RoomList[FindEmptyRoomIdx()];
													room->SetROOM(orderList[2], UserList[read.fd_array[i]]->GetName(), "2022-02-22", stoi(orderList[1]));
													room->SetUser(UserList[read.fd_array[i]]);
													SendMsg(UserList[read.fd_array[i]]->socket, "대화방이 개설되었습니다.\r\n");
												}
												else {
													SendMsg(UserList[read.fd_array[i]]->socket, "대화방 제목이 필요합니다.\r\n");
												}
											}
											else {
												SendMsg(UserList[read.fd_array[i]]->socket, "대화방 인원을 2-20명 사이로 입력해주세요.\r\n");
											}
										}
										else {
											SendMsg(UserList[read.fd_array[i]]->socket, "올바른 사용법은 O [최대인원] [방제목] 입니다.\r\n");
										}

									}
									else if (orderList[0] == "J")
									{
										//대화방 참가하기
										SendMsg(UserList[read.fd_array[i]]->socket, "** 올바른 사용법은 LOGIN [ID] 입니다.\r\n");
									}
									else if (orderList[0] == "X")
									{
										SendMsg(UserList[read.fd_array[i]]->socket, "** 올바른 사용법은 LOGIN [ID] 입니다.\r\n");
									}
								}
								if (UserList[read.fd_array[i]]->GetState() == State::lobby) {
									SendMsg(UserList[read.fd_array[i]]->socket, "명령어안내(H) 종료(X)\r\n");
									SendMsg(UserList[read.fd_array[i]]->socket, "선택>");
								}
								break;
							case State::room:
								SendMsg(UserList[read.fd_array[i]]->socket, (">" + msgString + "\r\n").c_str());
								break;
							default:
								break;
							}
							delete msg;
							UserList[read.fd_array[i]]->buffer.clear();
						}
						else {
							//명령 수집
							UserList[read.fd_array[i]]->buffer.push_back(c);
						}
					}
				}
			}
		}
	}



}

