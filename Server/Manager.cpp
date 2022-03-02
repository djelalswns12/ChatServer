#include "Manager.h"

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

void Manager::ServerON() {
	InitSocket();
	CreateSocket();
	SetSocketInfo();
	Bind();
	Listen();
	RoomList = vector<ROOM>(100);
}
void Manager::InitSocket() {
	if (WSAStartup(MAKEWORD(2, 2), &soData) != 0) {
		cout << "소켓 정보 로드 실패";
	}
}
void Manager::CreateSocket() {
	//PF_INET = IPV4
	//SOCK_STREAM = TCP / IP
	ServerSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_IP);
	if (ServerSocket == INVALID_SOCKET) {
		cout << "소켓 생성 실패\n";
	}
}
void Manager::SetSocketInfo() {
	//_serverAddr세팅
	memset(&soAddr, 0, sizeof(soAddr));
	soAddr.sin_family = AF_INET;
	soAddr.sin_addr.S_un.S_addr = htonl(INADDR_ANY); //IP설정
	soAddr.sin_port = htons(_port);
}
void Manager::Bind() {
	if (bind(ServerSocket, (SOCKADDR*)&soAddr, sizeof(soAddr)) == SOCKET_ERROR) {
		cout << "소켓 바인드 실패\n";
	}
}
void Manager::Listen() {
	if (listen(ServerSocket, 5) == SOCKET_ERROR) {
		cout << "소켓 리슨 실패\n";
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
			u->SendMsg(UserList[*tmp]->GetName() + "님이 나가셨습니다\r\n");
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
	user->SendMsg("명령어안내(H) 종료(X)\r\n");
	user->SendMsg("선택>");
}
void Manager::Login(USER* user, vector<string>& orderList) {
	/*
	닉네임 설정 후
	닉네임 리스트에 추가
	유저 상태 변경
	*/
	if (orderList.size() > 1 && orderList[1].length() > 0) {
		if (user->GetState() != State::auth) {
			//이미 로그인됨
			cout << "이미 로그인됨";
			return;
		}
		if (NameList.find(orderList[1]) == NameList.end()) {
			user->SetName(orderList[1]);
			NameList.insert(make_pair(orderList[1], user));
			user->SetState(State::lobby);
			user->SendMsg("-----------------------------------------------------\r\n 반갑습니다. 텍스트 채팅 서버 ver 0.1 입니다.\r\n 이용중 불편하신 점이 있으면 아래 이메일로 문의 바랍니다.\r\n 감사합니다.\r\n\t\tprogrammed & arranged by MIN JOON \r\n\t\temail:djelalswns12@naver.com\r\n-----------------------------------------------------\r\n");
			SendPrompt(user);
		}
		else {
			user->SendMsg("**아이디를 이미 사용중입니다.다른 아이디를 사용해주세요.\r\n");
		}
	}
	else {
		user->SendMsg("** 올바른 사용법은 LOGIN [ID] 입니다.\r\n");
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
	string s = "---------------------이용자 목록---------------------\r\n";
	for (auto iter = UserList.begin(); iter != UserList.end(); iter++) {
		s += " 이용자: " + (iter->second->GetName()) + "\t접속지: " + iter->second->GetIP() + to_string(iter->second->GetPort()) + "\r\n";;
	}
	s += "-----------------------------------------------------\r\n";
	user->SendMsg(s.c_str());
}
void Manager::H(USER* user, vector<string>& orderList) 
{
	user->SendMsg(
		"-----------------------------------------------------\r\nH\t\t\t명령어 안내\r\nUS\t\t\t이용자 목록 보기\r\nLT\t\t\t대화방 목록 보기\r\nST [방번호]\t\t대화방 정보 보기\r\nPF [상대방ID]\t\t이용자 정보 보기\r\nTO [상대방ID] [메시지]\t쪽지 보내기\r\nO  [최대인원] [방제목]\t대화방 만들기\r\nJ  [방번호]\t\t대화방 참여하기\r\nX\t\t\t끝내기\r\n-----------------------------------------------------\r\n"
	);
}
void Manager::H_(USER* user, vector<string>& orderList) 
{
	user->SendMsg(
		"-----------------------------------------------------\r\n/H\t\t\t명령어 안내\r\n/US\t\t\t이용자 목록 보기\r\n/LT\t\t\t대화방 목록 보기\r\n/ST [방번호]\t\t대화방 정보 보기\r\n/PF [상대방ID]\t\t이용자 정보 보기\r\n/TO [상대방ID] [메시지]\t쪽지 보내기\r\n/IN [상대방ID]\t\t초대하기\r\n/Q\t\t\t대화방 나가기\r\n/X\t\t\t끝내기\r\n-----------------------------------------------------\r\n"
	);
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
	string s = "-----------------------------------------------------\r\n";
	for (int i = 0; i < openRoomIdx.size(); i++) {
		s += "[\t" + to_string(openRoomIdx[i] + 1) + "]: (" + to_string(RoomList[openRoomIdx[i]].GetUsersSize()) + "/" + to_string(RoomList[openRoomIdx[i]].GetMaxCnt()) + ") " + RoomList[openRoomIdx[i]].GetName() + "\r\n";
	}
	s += "-----------------------------------------------------\r\n";
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
				s = user->GetName() + "님이 입장했습니다.\r\n";
				for (USER* u : user->GetmyRoom()->GetUsers()) {
					u->SendMsg(s.c_str());
				}
			}
			else {
				s = "** 인원이 꽉차서 참여할 수 없습니다.\r\n";
				user->SendMsg(s.c_str());
			}
		}
		else {
			s = "** 존재하지 않는 대화방입니다.\r\n";
			user->SendMsg(s.c_str());
		}
	}
	else {
		s = "** 올바른 사용법은 J [방번호] 입니다.\r\n";
		user->SendMsg(s.c_str());
	}

}
void Manager::O(USER* user, vector<string>& orderList)
{
	//대화방 만들기
	string s;
	if (orderList.size() > 1)
	{
		if (isNumber(orderList[1]) == true && stoi(orderList[1]) >= 2 && stoi(orderList[1]) <= 20) {
			if (orderList.size() > 2 && orderList[2].length() > 2) {
				int idx = FindEmptyRoomIdx();
				ROOM* room = &RoomList[idx];
				room->SetROOM(orderList[2], user->GetName(), GetNowTime(), stoi(orderList[1]), idx);
				room->SetUser(user);
				s = "대화방이 개설되었습니다.\r\n" + user->GetName() + "님이 입장했습니다.\r\n";
			}
			else {
				s = "대화방 제목이 필요합니다.\r\n";
			}
		}
		else {
			s = "대화방 인원을 2-20명 사이로 입력해주세요.\r\n";
		}
	}
	else {
		s = "올바른 사용법은 O [최대인원] [방제목] 입니다.\r\n";
	}
	user->SendMsg(s.c_str());
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
					s = "** 쪽지를 보냈습니다.\r\n";
					string mail;
					mail = "\r\n# " + user->GetName() + "님의 쪽지 ==>" + orderList[2] + "\r\n";
					NameList[orderList[1]]->SendMsg(mail.c_str());
				}
				else {
					s = "** 자기 자신에게는 보낼 수 없습니다.\r\n";
				}
			}
			else {
				s = "** 이용자를 찾을 수 없습니다.\r\n";
			}
		}
		else {
			s = "** 메시지 내용이 없습니다.\r\n";
		}
	}
	else {
		s = "** 올바른 사용법은 TO [상대방ID] [매시지] 입니다.\r\n";
	}
	user->SendMsg(s.c_str());
}
void Manager::ST(USER* user, vector<string>& orderList)
{
	//** 올바른 사용법은 ST [방번호] 입니다.  
										//**존재하지 않는 대화방입니다.
	if (orderList.size() > 1 && isNumber(orderList[1])) {
		int roomIdx = stoi(orderList[1]) - 1;
		if (RoomList[roomIdx].GetOpen()) {
			string s = "-----------------------------------------------------\r\n";
			//룸 정보
			s += "[\t" + to_string(roomIdx + 1) + "]: (" + to_string(RoomList[roomIdx].GetUsersSize()) + "/" + to_string(RoomList[roomIdx].GetMaxCnt()) + ") " + RoomList[roomIdx].GetName() + "\r\n";
			s += " 개설시간: " + RoomList[roomIdx].GetOpenTime() + "\r\n";
			for (USER* u : RoomList[roomIdx].GetUsers()) {
				s += " 참여자: " + u->GetName() + "\t" + "참여시간: " + u->GetJoinTime() + "\r\n";
			}
			//참여자 정보
			s += "-----------------------------------------------------\r\n";
			user->SendMsg(s.c_str());
		}
		else {
			user->SendMsg("**존재하지 않는 대화방입니다.\r\n");
		}
	}
	else {
		user->SendMsg("** 올바른 사용법은 ST [방번호] 입니다.\r\n");
	}
}
void Manager::PF(USER* user, vector<string>& orderList)
{
	string s;
	//님은 현재 1번 방에 참여중입니다.
	//님을 찾을 수 없습니다. 
	if (orderList.size() > 1) {
		if (NameList.find(orderList[1]) != NameList.end())
		{
			if (NameList[orderList[1]]->GetState() == State::lobby)
			{
				s = "** " + NameList[orderList[1]]->GetName() + "님은 현재 대기실에 있습니다.\r\n" + "** 접속지: " + NameList[orderList[1]]->GetIP() + ":" + to_string(NameList[orderList[1]]->GetPort()) + "\r\n";
			}
			else if (NameList[orderList[1]]->GetState() == State::room) {
				s = "** " + NameList[orderList[1]]->GetName() + "님은 현재 " + to_string((NameList[orderList[1]]->GetmyRoom()->GetNumber()) + 1) + "번 방에 있습니다.\r\n" + "** 접속지: " + NameList[orderList[1]]->GetIP() + ":" + to_string(NameList[orderList[1]]->GetPort()) + "\r\n";
			}
			user->SendMsg(s.c_str());
		}
		else
		{
			s = "** " + orderList[1] + "님을 찾을 수 없습니다.\r\n";
			user->SendMsg(s.c_str());
		}
	}
	else
	{
		if (user->GetState() == State::lobby)
		{
			s = "** " + user->GetName() + "님은 현재 대기실에 있습니다.\r\n" + "** 접속지: " + user->GetIP() + ":" + to_string(user->GetPort()) + "\r\n";
		}
		user->SendMsg(s.c_str());
	}
}
void Manager::Q(USER* user, vector<string>& orderList)
{
	DisConnectRoom(&user->socket);
	SendPrompt(user);
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
					s = "** 초대 요청을 했습니다.\r\n";
					string mail;
					mail = "\r\n# " + user->GetName() + "님의 초대장이 도착했습니다.\r\n";
					NameList[orderList[1]]->SendMsg(mail.c_str());
				}
				else {
					s = "** 이미 같은 방에 있습니다.\r\n";
				}
			}
			else {
				s = "** 자기 자신은 초대할 수 없습니다.\r\n";
			}
		}
		else {
			s="** 이용자를 찾을 수 없습니다.\r\n";
		}
	}
	else {
		s = "** 올바른 사용법은 IN [상대방ID] 입니다.\r\n";
	}
	user->SendMsg(s.c_str());
}

