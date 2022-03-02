#include "Manager.h"
bool Manager::ExcuteOrder(USER* u, vector<string>& vec) {
	if (vec.size() <= 0)
	{
		return false;
	}
	if (stoi(DB.GetData(vec[0], "state")) == (int)(u->GetState())) {
		if (OrderFuncs.find(vec[0]) != OrderFuncs.end()) {
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

vector<string> Manager::split(string ids, string target, int cnt) {
	vector<string> names;
	size_t cur, pre = 0;
	cur = ids.find(target);
	while (cur != string::npos) {
		string substring = ids.substr(pre, cur - pre);
		if (substring.length() > 0) {
			names.push_back(substring);
			cnt--;
		}
		pre = cur + target.length();
		cur = ids.find(target, pre);
		if (cnt <= 0) {
			break;
		}
	}
	if (cnt > 0) {
		if (ids.substr(pre, cur - pre).length() > 0) {
			names.push_back(ids.substr(pre, cur - pre));
		}
	}
	else {
		names.push_back(ids.substr(pre, ids.length() - pre));
	}

	return names;
}

vector<string> Manager::split(string ids, string target) {
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
bool Manager::isNumber(string s) {
	for (char c : s) {
		if (isdigit(c) == 0) {
			return false;
		}
	}
	return true;
}

void Manager::ServerON(string data) {
	DB.ReadData(data);
	SetOrder(DB.GetOrderData());
	InitSocket();
	CreateSocket();
	SetSocketInfo();
	Bind();
	Listen();
	RoomList = vector<ROOM>(100); // ������� �������� ���ϰ� �߰� ����
}
void Manager::InitSocket() {
	if (WSAStartup(MAKEWORD(2, 2), &soData) != 0) {
		cout << "���� ���� �ε� ����";
	}
}
void Manager::CreateSocket() {
	//PF_INET = IPV4
	//SOCK_STREAM = TCP / IP
	ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	if (ServerSocket == INVALID_SOCKET) {
		cout << "���� ���� ����\n";
	}
}
void Manager::SetSocketInfo() {
	//_serverAddr����
	memset(&soAddr, 0, sizeof(soAddr));
	soAddr.sin_family = AF_INET;
	soAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //IP����
	soAddr.sin_port = htons(_port);
}
void Manager::Bind() {
	if (bind(ServerSocket, (SOCKADDR*)&soAddr, sizeof(soAddr)) == SOCKET_ERROR) {
		cout << "���� ���ε� ����\n";
	}
}
void Manager::Listen() {
	if (listen(ServerSocket, 5) == SOCKET_ERROR) {
		cout << "���� ���� ����\n";
	}
	cout << "Server ON\n\n";
}
void Manager::Print(string s) {
	cout << s;
}


void Manager::DisConnectRoom(SOCKET* tmp) {

	if (UserList.at(*tmp)->GetmyRoom() != nullptr) {
		for (USER* u : UserList[*tmp]->GetmyRoom()->GetUsers())
		{
			if (u->socket == *tmp && u->GetFin()) {
				continue;
			}
				u->SendMsg(UserList[*tmp]->GetName() + "���� �����̽��ϴ�\r\n");
		}
		UserList.at(*tmp)->GetmyRoom()->DisConnectUser(UserList.at(*tmp));
		UserList.at(*tmp)->SetmyRoom(nullptr, "");

	}
}

void Manager::DisConnect(SOCKET* tmp) {
	DisConnectRoom(tmp);
	NameList.erase(UserList.at(*tmp)->GetName());
	UserList.erase(*tmp);
	shutdown(*tmp, SD_BOTH);
	closesocket(*tmp);
	//cout << UserList.size() << "\n";
}
void Manager::SendPrompt(USER* user) {
	user->SendMsg("���ɾ�ȳ�(H) ����(X)\r\n");
	user->SendMsg("����>");
}
void Manager::Login(USER* user, vector<string>& orderList) {
	if (orderList.size() > 1 && orderList[1].length() > 0) {
		if (user->GetState() != State::auth) {
			return;
		}
		if (NameList.find(orderList[1]) == NameList.end()) {
			user->SetName(orderList[1]);
			NameList.insert(make_pair(orderList[1], user));
			user->SetState(State::lobby);
			user->SendMsg(DB.GetData(orderList[0], "comment0"));
		}
		else {
			user->SendMsg(DB.GetData(orderList[0], "comment1"));
		}
	}
	else {
		user->SendMsg(DB.GetData(orderList[0], "comment0"));
	}
}

int Manager::FindEmptyRoomIdx() {
	for (int i = 0; i < RoomList.size(); i++)
	{
		if (RoomList[i].GetOpen() == false)
		{
			return i;
		}
	}
	return -1;
}

void Manager::US(USER* user, vector<string>& orderList) 
{
	string s = "";
	for (auto iter = UserList.begin(); iter != UserList.end(); iter++) {
		s += DB.AssignData(DB.GetData(orderList[0], "comment0"), vector<string>{iter->second->GetName(), iter->second->GetIP(), to_string(iter->second->GetPort())});
	}
	user->SendMsg(s.c_str());
}
void Manager::H(USER* user, vector<string>& orderList) 
{
	user->SendMsg(DB.GetData(orderList[0], "comment0"));
}
void Manager::H_(USER* user, vector<string>& orderList) 
{
	user->SendMsg(DB.GetData(orderList[0], "comment0"));
}
void Manager::LT(USER* user, vector<string>& orderList) 
{
	vector<int> openRoomIdx;
	int cnt = 0;
	for (auto iter = RoomList.begin(); iter != RoomList.end(); iter++) {
		if (iter->GetOpen() == true) {
			openRoomIdx.push_back(cnt);
		}
		cnt++;
	}
	string s = "";
	for (int i = 0; i < openRoomIdx.size(); i++) {
		s += DB.AssignData(DB.GetData(orderList[0], "comment0"), vector<string>{to_string(openRoomIdx[i] + 1), to_string(RoomList[openRoomIdx[i]].GetUsersSize()) , to_string(RoomList[openRoomIdx[i]].GetMaxCnt()), RoomList[openRoomIdx[i]].GetName()});
	}
	user->SendMsg(s.c_str());
}
void Manager::J(USER* user, vector<string>& orderList)
{
	string s;
	if (orderList.size() > 1)
	{
		if (isNumber(orderList[1]) && RoomList[stoi(orderList[1]) - 1].GetOpen()) {
			if (RoomList[stoi(orderList[1]) - 1].isFull() == false) {
				RoomList[stoi(orderList[1]) - 1].SetUser(user);
				s = DB.AssignData(DB.GetData(orderList[0], "comment0"), vector<string>{user->GetName() });
				for (USER* u : user->GetmyRoom()->GetUsers()) {
					u->SendMsg(s.c_str());
				}
				return;
			}
			else {
				s = DB.GetData(orderList[0], "comment1");
			}
		}
		else {
			s = DB.GetData(orderList[0], "comment2");
		}
	}
	else {
		s = DB.GetData(orderList[0], "comment3");
	}
	user->SendMsg(s.c_str());
}
void Manager::O(USER* user, vector<string>& orderList)
{
	//��ȭ�� �����
	string s;
	if (orderList.size() > 1)
	{
		if (isNumber(orderList[1]) == true && stoi(orderList[1]) >= 2 && stoi(orderList[1]) <= 20) {
			if (orderList.size() > 2 && orderList[2].length() > 2) {
				int idx = FindEmptyRoomIdx();
				ROOM* room = &RoomList[idx];
				room->SetROOM(orderList[2], user->GetName(), GetNowTime(), stoi(orderList[1]), idx);
				room->SetUser(user);
				s = "��ȭ���� �����Ǿ����ϴ�.\r\n" + user->GetName() + "���� �����߽��ϴ�.\r\n";
			}
			else {
				s = "��ȭ�� ������ �ʿ��մϴ�.\r\n";
			}
		}
		else {
			s = "��ȭ�� �ο��� 2-20�� ���̷� �Է����ּ���.\r\n";
		}
	}
	else {
		s = "�ùٸ� ������ O [�ִ��ο�] [������] �Դϴ�.\r\n";
	}
	user->SendMsg(s.c_str());
}void Manager::X(USER* user, vector<string>& orderList)
{
	//��ȭ�� �����
	user->SendMsg("** ����޼���.\r\n");
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
					s = "** ������ ���½��ϴ�.\r\n";
					string mail;
					mail = "\r\n# " + user->GetName() + "���� ���� ==>" + orderList[2] + "\r\n";
					NameList[orderList[1]]->SendMsg(mail.c_str());
				}
				else {
					s = "** �ڱ� �ڽſ��Դ� ���� �� �����ϴ�.\r\n";
				}
			}
			else {
				s = "** �̿��ڸ� ã�� �� �����ϴ�.\r\n";
			}
		}
		else {
			s = "** �޽��� ������ �����ϴ�.\r\n";
		}
	}
	else {
		s = "** �ùٸ� ������ TO [����ID] [�Ž���] �Դϴ�.\r\n";
	}
	user->SendMsg(s.c_str());
}
void Manager::ST(USER* user, vector<string>& orderList)
{
	string s;
	if (orderList.size() > 1 && isNumber(orderList[1])) {
		int roomIdx = stoi(orderList[1]) - 1;
		if (RoomList[roomIdx].GetOpen()) {

			//�� ����
			s+= DB.AssignData(DB.GetData(orderList[0], "comment0"), vector<string>{ to_string(roomIdx + 1), to_string(RoomList[roomIdx].GetUsersSize()), to_string(RoomList[roomIdx].GetMaxCnt()), RoomList[roomIdx].GetName() });

			//���� �ð�
			s +=  DB.AssignData(DB.GetData(orderList[0], "comment1"), vector<string>{RoomList[roomIdx].GetOpenTime() });

			for (USER* u : RoomList[roomIdx].GetUsers()) {
				//������ ����
				s+= DB.AssignData(DB.GetData(orderList[0], "comment2"), vector<string>{u->GetName(), u->GetJoinTime()});

			}
			s += DB.GetData(orderList[0], "comment3");
		}
		else {
			s= DB.GetData(orderList[0], "comment4");
		}
	}
	else {
		s= DB.GetData(orderList[0], "comment5");
	}

	user->SendMsg(s.c_str());
}
void Manager::PF(USER* user, vector<string>& orderList)
{
	string s;
	//���� ���� 1�� �濡 �������Դϴ�.
	//���� ã�� �� �����ϴ�. 
	if (orderList.size() > 1) {
		if (NameList.find(orderList[1]) != NameList.end())
		{
			if (NameList[orderList[1]]->GetState() == State::lobby)
			{
				s = "** " + NameList[orderList[1]]->GetName() + "���� ���� ���ǿ� �ֽ��ϴ�.\r\n" + "** ������: " + NameList[orderList[1]]->GetIP() + ":" + to_string(NameList[orderList[1]]->GetPort()) + "\r\n";
			}
			else if (NameList[orderList[1]]->GetState() == State::room) {
				s = "** " + NameList[orderList[1]]->GetName() + "���� ���� " + to_string((NameList[orderList[1]]->GetmyRoom()->GetNumber()) + 1) + "�� �濡 �ֽ��ϴ�.\r\n" + "** ������: " + NameList[orderList[1]]->GetIP() + ":" + to_string(NameList[orderList[1]]->GetPort()) + "\r\n";
			}
			user->SendMsg(s.c_str());
		}
		else
		{
			s = "** " + orderList[1] + "���� ã�� �� �����ϴ�.\r\n";
			user->SendMsg(s.c_str());
		}
	}
	else
	{
		if (user->GetState() == State::lobby)
		{
			s = "** " + user->GetName() + "���� ���� ���ǿ� �ֽ��ϴ�.\r\n" + "** ������: " + user->GetIP() + ":" + to_string(user->GetPort()) + "\r\n";
		}
		user->SendMsg(s.c_str());
	}
}
void Manager::Q(USER* user, vector<string>& orderList)
{
	DisConnectRoom(&user->socket);
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
				if (user->GetmyRoom() != NameList[orderList[1]]->GetmyRoom()) {
					s = "** �ʴ� ��û�� �߽��ϴ�.\r\n";
					string mail;
					mail = "\r\n# " + user->GetName() + "���� �ʴ����� �����߽��ϴ�.\r\n";
					NameList[orderList[1]]->SendMsg(mail.c_str());
				}
				else {
					s = "** �̹� ���� �濡 �ֽ��ϴ�.\r\n";
				}
			}
			else {
				s = "** �ڱ� �ڽ��� �ʴ��� �� �����ϴ�.\r\n";
			}
		}
		else {
			s="** �̿��ڸ� ã�� �� �����ϴ�.\r\n";
		}
	}
	else {
		s = "** �ùٸ� ������ IN [����ID] �Դϴ�.\r\n";
	}
	user->SendMsg(s.c_str());
}
