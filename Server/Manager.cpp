#include "Manager.h"
bool Manager::ExcuteOrder(USER* u, vector<string>& vec) 
{
	if (vec.size() <= 0)
	{
		return false;
	}
	if (stoi(DB->GetData(vec[0], "state")) == (int)(u->GetState())) 
	{
		if (OrderFuncs.find(vec[0]) != OrderFuncs.end()) 
		{
			(this->*(Manager::OrderFuncs[vec[0]]))(u, vec);
			return true;
		}
	}
	return false;
}
string Manager::GetNowTime()
{
	time_t timer = time(NULL);
	struct tm* t = localtime(&timer);
	string hour = to_string(t->tm_hour);
	string min = to_string(t->tm_min);
	string sec = to_string(t->tm_sec);

	hour = hour.length() < 2 ? "0" + hour : hour;
	min = min.length() < 2 ? "0" + min : min;
	sec = sec.length() < 2 ? "0" + sec : sec;

	return  hour + ":" + min + ":" + sec;
}

vector<string> Manager::Split(const string& ids, string target, int cnt) 
{
	vector<string> names;
	size_t cur, pre = 0;
	cur = ids.find(target);
	while (cur != string::npos) 
	{
		string substring = ids.substr(pre, cur - pre);
		if (substring.length() > 0) 
		{
			names.push_back(substring);
			cnt--;
		}
		pre = cur + target.length();
		cur = ids.find(target, pre);
		if (cnt <= 0) {
			break;
		}
	}
	if (cnt > 0) 
	{
		if (ids.substr(pre, cur - pre).length() > 0) 
		{
			names.push_back(ids.substr(pre, cur - pre));
		}
	}
	else 
	{
		names.push_back(ids.substr(pre, ids.length() - pre));
	}

	return names;
}

vector<string> Manager::Split(const string& ids, string target) 
{
	vector<string> names;
	size_t cur, pre = 0;
	cur = ids.find(target);
	while (cur != string::npos) 
	{
		string substring = ids.substr(pre, cur - pre);
		if (substring.length() > 0) 
		{
			names.push_back(substring);
		}
		pre = cur + target.length();
		cur = ids.find(target, pre);
	}
	if (ids.substr(pre, cur - pre).length() > 0) 
	{
		names.push_back(ids.substr(pre, cur - pre));
	}
	return names;
}
bool Manager::IsNumber(string s) 
{
	for (char c : s) {
		if (isdigit(c) == 0) {
			return false;
		}
	}
	return true;
}

void Manager::ServerON(string data) 
{
	DB = new DataParse();
	DB->ReadData(data);
	SetOrder(DB->GetOrderData());
	InitSocket();
	CreateSocket();
	SetSocketInfo();
	Bind();
	Listen();
}
void Manager::InitSocket() 
{
	if (WSAStartup(MAKEWORD(2, 2), &SocketData) != 0) 
	{
		cout << "소켓 정보 로드 실패";
	}
}
void Manager::CreateSocket() 
{
	//PF_INET = IPV4
	//SOCK_STREAM = TCP / IP
	ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	if (ServerSocket == INVALID_SOCKET) 
	{
		cout << "소켓 생성 실패\n";
	}
}
void Manager::SetSocketInfo() 
{
	//_serverAddr세팅
	memset(&SocketAddr, 0, sizeof(SocketAddr));
	SocketAddr.sin_family = AF_INET;
	SocketAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //IP설정
	SocketAddr.sin_port = htons(Port);
}
void Manager::Bind() 
{
	if (bind(ServerSocket, (SOCKADDR*)&SocketAddr, sizeof(SocketAddr)) == SOCKET_ERROR) 
	{
		cout << "소켓 바인드 실패\n";
	}
}
void Manager::Listen() {
	if (listen(ServerSocket, 5) == SOCKET_ERROR) 
	{
		cout << "소켓 리슨 실패\n";
	}
	cout << " Server ON\n\n- Log\n";
}
void Manager::Print(const string& s) {
	cout << s;
}
void Manager::SetOrder(vector<pair<string, string>> data)
{
	for (auto v : data) 
	{
		OrderFuncs.insert(make_pair(v.first, OrderFuncsList[stoi(v.second)]));
	}
}

void Manager::DisConnectRoom(SOCKET* tmp) 
{
	if (UserList.at(*tmp)->GetmyRoom() != nullptr) 
	{
		for (USER* u : UserList[*tmp]->GetmyRoom()->GetUsers())
		{
			if (u->Socket == *tmp && u->GetFin()) 
			{
				continue;
			}
			string s = UserList[*tmp]->GetName() + "님이 나가셨습니다\r\n";
			u->SendMsg("<H>UM<H>" + s + "<H>UM<H>");
		}
		UserList.at(*tmp)->GetmyRoom()->DisConnectUser(UserList.at(*tmp));
		UserList.at(*tmp)->SetmyRoom(nullptr, "");
	}
}
void Manager::InsertUser(USER* user)
{
	UserList.insert(make_pair(user->Socket, user));
	//UserChangeEvent();
}
void Manager::RemoveUser(SOCKET* tmp)
{
	NameList.erase(UserList.at(*tmp)->GetName());
	UserList.erase(*tmp);
	UserChangeEvent();
}
void Manager::UserChangeEvent()
{
	// 유저 정보를 브로드 캐스트 한다.
	//cout << "\n \t유저 데이터 변화감지 , 브로드캐스트\n";
	vector<string> s1,s2;
	s1.push_back(string("US"));
	s2.push_back(string("/US"));
	for (auto u : UserList) {
		if (u.second->GetIsUE()) 
		{
			if (u.second->GetState() == EState::Lobby || u.second->GetState() == EState::Auth)
			{
				ExcuteOrder(u.second, s1);
			}
			else {
				ExcuteOrder(u.second, s2);
			}
		}
	}
}
void Manager::RoomChangeEvent()
{
	// 룸 정보를 브로드 캐스트 한다.
	//cout << "\n \t룸 데이터 변화감지 , 브로드캐스트\n";
	vector<string> s1,s2;
	s1.push_back(string("LT"));
	s2.push_back(string("/LT"));
	for (auto u : UserList) {
		if (u.second->GetIsUE())
		{
			if (u.second->GetState() == EState::Lobby || u.second->GetState() == EState::Auth)
			{
				ExcuteOrder(u.second, s1);
			}
			else {
				ExcuteOrder(u.second, s2);
			}
		}
	}
}
void Manager::DisConnect(SOCKET* tmp) 
{
	USER *user= UserList[*tmp];
	DisConnectRoom(tmp);
	RemoveUser(tmp);
	shutdown(*tmp, SD_BOTH);
	closesocket(*tmp);
	delete user;
	//cout << UserList.size() << "\n";
}
void Manager::SendPrompt(USER* user) {
	string s = "명령어안내(H) 종료(X)\r\n선택 > ";
	Msg(user,s);
}
void Manager::Msg(USER* user, string& data)
{
	string s = "<H>MS<H>"+data;
	s += "<H>MS<H>";
	user->SendMsg(s);
}
void Manager::Login(USER* user, vector<string>& orderList)
{
	string s = "";
	if (orderList.size() > 1 && orderList[1].length() > 0)
	{
		if (user->GetState() != EState::Auth)
		{
			return;
		}
		//언리얼 유저인지 구분

		if (orderList[0].find("/U") != string::npos) {
			user->SetIsUE(true);
		}
		else {
			user->SetIsUE(false);
		}
		if (NameList.find(orderList[1]) == NameList.end())
		{
			user->SetName(orderList[1]);
			NameList.insert(make_pair(orderList[1], user));
			user->SetState(EState::Lobby);
			s += "<H>LS<H>";
			s += DB->GetData(orderList[0], "comment0");
			s += "<H>LS<H>";
		}
		else
		{
			s += "<H>LF<H>";
			s += DB->GetData(orderList[0], "comment1");
			s += "<H>LF<H>";
		}
	}
	else 
	{
		s += DB->GetData(orderList[0], "comment2");
	}
	user->SendMsg(s);
	UserChangeEvent();
	RoomChangeEvent();
}

int Manager::FindEmptyRoomIdx() 
{
	for (int i = 0; i < RoomList.size(); i++)
	{
		if (RoomList[i].GetOpen() == false)
		{
			return i;
		}
	}
	RoomList.push_back(ROOM());
	return RoomList.size()-1;
}

void Manager::US(USER* user, vector<string>& orderList) 
{
	string s = "<H>US<H>";
	s += DB->GetData(orderList[0], "comment1");
	for (auto iter = UserList.begin(); iter != UserList.end(); iter++) 
	{
		if (iter->second->GetState() == EState::Lobby) 
		{
			s += DB->AssignData(DB->GetData(orderList[0], "comment0"), vector<string>{iter->second->GetName(), iter->second->GetIP(), to_string(iter->second->GetPort())});
		}
		else if (iter->second->GetState() == EState::Room) 
		{
			s+=DB->AssignData(DB->GetData(orderList[0], "comment3"), vector<string>{iter->second->GetName(),to_string(iter->second->GetmyRoom()->GetNumber()) ,iter->second->GetIP(), to_string(iter->second->GetPort())});
		}
	}
	s += DB->GetData(orderList[0], "comment2");
	s += "<H>US<H>";
	user->SendMsg(s);
}
void Manager::H(USER* user, vector<string>& orderList) 
{
	user->SendMsg(DB->GetData(orderList[0], "comment0"));
}
void Manager::H_(USER* user, vector<string>& orderList) 
{
	user->SendMsg(DB->GetData(orderList[0], "comment0"));
}
void Manager::LT(USER* user, vector<string>& orderList) 
{
	vector<int> openRoomIdx;
	int cnt = 0;
	for (auto iter = RoomList.begin(); iter != RoomList.end(); iter++) 
	{
		if (iter->GetOpen() == true) 
		{
			openRoomIdx.push_back(cnt);
		}
		cnt++;
	}
	string s = "<H>LT<H>";
	s += DB->GetData(orderList[0], "comment1");
	for (int i = 0; i < openRoomIdx.size(); i++) 
	{
		s += DB->AssignData(DB->GetData(orderList[0], "comment0"), vector<string>{to_string(openRoomIdx[i] + 1), to_string(RoomList[openRoomIdx[i]].GetUsersSize()) , to_string(RoomList[openRoomIdx[i]].GetMaxCnt()), RoomList[openRoomIdx[i]].GetName()});
	}
	s += DB->GetData(orderList[0], "comment1");
	s += "<H>LT<H>";
	user->SendMsg(s);
}
void Manager::J(USER* user, vector<string>& orderList)
{
	string s;
	if (orderList.size() > 1)
	{
		if (IsNumber(orderList[1]) && RoomList[stoi(orderList[1]) - 1].GetOpen()) 
		{
			if (RoomList[stoi(orderList[1]) - 1].IsFull() == false) 
			{
				RoomList[stoi(orderList[1]) - 1].SetUser(user);
				s = DB->AssignData(DB->GetData(orderList[0], "comment0"), vector<string>{user->GetName() });
				for (USER* u : user->GetmyRoom()->GetUsers()) 
				{
					if (u != user) {
						u->SendMsg("<H>UM<H>"+s+"<H>UM<H>");
					}
					else {
						u->SendMsg("<H>RS<H>"+s+"<H>RS<H>");
					}
				}
				return;
			}
			else 
			{
				//인원초과로 인한 입장 불가.
				s += "<H>PM<H>";
				s += DB->GetData(orderList[0], "comment1");
				s += "<H>PM<H>";
			}
		}
		else 
		{
			//없는방에 입장 시도.
			s += "<H>PM<H>";
			s += DB->GetData(orderList[0], "comment2");
			s += "<H>PM<H>";
		}
	}
	else 
	{
		s += "<H>PM<H>";
		s += DB->GetData(orderList[0], "comment3");
		s += "<H>PM<H>";
	}
	user->SendMsg(s);
}
void Manager::O(USER* user, vector<string>& orderList)
{
	//대화방 만들기
	string s;
	if (orderList.size() > 1)
	{
		if (IsNumber(orderList[1]) == true && stoi(orderList[1]) >= 2 && stoi(orderList[1]) <= 20) 
		{
			if (orderList.size() > 2 && orderList[2].length() > 2) 
			{
				int idx = FindEmptyRoomIdx();
				ROOM* room = &RoomList[idx];
				room->SetRoom(orderList[2], user->GetName(), GetNowTime(), stoi(orderList[1]), idx);
				room->SetUser(user);
				s += "<H>RS<H>";
				s += DB->AssignData(DB->GetData(orderList[0], "comment0"), vector<string>{user->GetName()});
				s += "<H>RS<H>";
			}
			else {
				s += "<H>PM<H>";
				s += DB->GetData(orderList[0], "comment1");
				s += "<H>PM<H>";
			}
		}
		else {
			s += "<H>PM<H>";
			s += DB->GetData(orderList[0], "comment2");
			s += "<H>PM<H>";
		}
	}
	else 
	{
		s += "<H>PM<H>";
		s += DB->GetData(orderList[0], "comment3");
		s += "<H>PM<H>";
	}
	user->SendMsg(s);
}
void Manager::X(USER* user, vector<string>& orderList)
{
	//대화방 만들기
	string s = "** 종료메세지.\r\n";
	user->SendMsg(s);
	user->SetFin(true);
}
void Manager::TO(USER* user, vector<string>& orderList)
{
	string s;
	if (orderList.size() > 1)
	{
		if (orderList.size() > 2)
		{
			if (NameList.find(orderList[1]) != NameList.end())
			{
				if (orderList[1] != user->GetName())
				{
					s = DB->GetData(orderList[0], "comment0");
					string mail;
					mail = "<H>PM<H>\r\n# " + user->GetName() + "님의 쪽지 ==>" + orderList[2] + "\r\n<H>PM<H>";
					NameList[orderList[1]]->SendMsg(mail);
				}
				else 
				{
					s = "<H>PM<H>" + DB->GetData(orderList[0], "comment1")+"<H>PM<H>";
				}
			}
			else 
			{
				s = "<H>MS<H>" + DB->GetData(orderList[0], "comment2") + "<H>MS<H>";
			}
		}
		else 
		{
			s = "<H>MS<H>" + DB->GetData(orderList[0], "comment3") + "<H>MS<H>";
		}
	}
	else 
	{
		s = "<H>MS<H>" + DB->GetData(orderList[0], "comment4") + "<H>MS<H>";
	}
	user->SendMsg(s);
}
void Manager::ST(USER* user, vector<string>& orderList)
{
	string s;
	if (orderList.size() > 1 && IsNumber(orderList[1])) 
	{
		int roomIdx = stoi(orderList[1]) - 1;
		
		if (RoomList.size()<=0)
		{
			s = DB->GetData(orderList[0], "comment4");
			Msg(user, s);
			return;
		}
		if (RoomList[roomIdx].GetOpen()) 
		{
			//룸 정보
			s+="<H>ST<H>"+DB->AssignData(DB->GetData(orderList[0], "comment0"), vector<string>{ to_string(roomIdx + 1), to_string(RoomList[roomIdx].GetUsersSize()), to_string(RoomList[roomIdx].GetMaxCnt()), RoomList[roomIdx].GetName() });
			//개설 시간
			s +=  DB->AssignData(DB->GetData(orderList[0], "comment1"), vector<string>{RoomList[roomIdx].GetOpenTime() });

			for (USER* u : RoomList[roomIdx].GetUsers()) 
			{
				//참여자 정보
				s+= DB->AssignData(DB->GetData(orderList[0], "comment2"), vector<string>{u->GetName(), u->GetJoinTime()});

			}
			s += DB->GetData(orderList[0], "comment3")+"<H>ST<H>";
			cout << s;
		}
		else 
		{
			s= DB->GetData(orderList[0], "comment4");
			Msg(user, s);
			return;
		}
	}
	else 
	{
		s= DB->GetData(orderList[0], "comment5");
		Msg(user, s);
		return;
	}
	user->SendMsg(s);
}
void Manager::PF(USER* user, vector<string>& orderList)
{
	string s;
	//님은 현재 1번 방에 참여중입니다.
	//님을 찾을 수 없습니다. 
	if (orderList.size() > 1) {
		if (NameList.find(orderList[1]) != NameList.end())
		{
			if (NameList[orderList[1]]->GetState() == EState::Lobby)
			{
				s= DB->AssignData(DB->GetData(orderList[0], "comment0"), vector<string>{NameList[orderList[1]]->GetName(), NameList[orderList[1]]->GetIP(), to_string(NameList[orderList[1]]->GetPort()) });
			}
			else if (NameList[orderList[1]]->GetState() == EState::Room) {
				s = DB->AssignData(DB->GetData(orderList[0], "comment1"), vector<string>{ NameList[orderList[1]]->GetName(), to_string((NameList[orderList[1]]->GetmyRoom()->GetNumber()) + 1), NameList[orderList[1]]->GetIP(), to_string(NameList[orderList[1]]->GetPort()) });
			}
		}
		else
		{
			s = DB->AssignData(DB->GetData(orderList[0], "comment2"), vector<string>{orderList[1]});
		}
	}
	else
	{
		if (user->GetState() == EState::Lobby)
		{
			s = DB->AssignData(DB->GetData(orderList[0], "comment0"), vector<string>{ user->GetName(), user->GetIP(), to_string(user->GetPort()) });
		}
	}
	user->SendMsg(s);
}
void Manager::Q(USER* user, vector<string>& orderList)
{
	DisConnectRoom(&user->Socket);
}
void Manager::IN_(USER* user, vector<string>& orderList)
{
	string s;
	if (orderList.size() > 1)
	{
		if (NameList.find(orderList[1]) != NameList.end())
		{
			if (orderList[1] != user->GetName())
			{
				if (user->GetmyRoom() != NameList[orderList[1]]->GetmyRoom()) 
				{
					s = "<H>PM<H>"+DB->GetData(orderList[0], "comment0")+ "<H>PM<H>";
					string data = "<H>PM<H>" + DB->AssignData(DB->GetData(orderList[0], "comment5"), vector<string>{ user->GetName()}) + "<H>PM<H>";
					NameList[orderList[1]]->SendMsg(data);
				}
				else 
				{
					s = "<H>PM<H>" + DB->GetData(orderList[0], "comment1") + "<H>PM<H>";
				}
			}
			else 
			{
				s = "<H>PM<H>" + DB->GetData(orderList[0], "comment2") + "<H>PM<H>";
			}
		}
		else 
		{
			s = "<H>PM<H>" + DB->GetData(orderList[0], "comment3") + "<H>PM<H>";
		}
	}
	else 
	{
		s = "<H>PM<H>" + DB->GetData(orderList[0], "comment4") + "<H>PM<H>";
	}
	user->SendMsg(s);
}
string Manager::ForTelnetData(string str)
{
	vector<string> src = Manager::getIncetance().Split(str, "@/n");
	string data = "";
	for (string s : src)
	{
		data += s;
	}
	return data;
}
string Manager::ForUE4Data(string str)
{
	vector<string> src = Manager::getIncetance().Split(str, "@/n");
	string data = "";
	for (string s : src)
	{
		data+= s+"\n";
	}
	return data;
}
