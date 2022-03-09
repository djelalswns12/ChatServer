#include "Manager.h"
#include "USER.h"
#include "ROOM.h"
#include "DataParse.h"
#include <tchar.h>

using namespace std;
wchar_t* ConverCtoWC(const char* str)
{
	//wchar_t형 변수 선언
	wchar_t* pStr;
	//멀티 바이트 크기 계산 길이 반환
	int strSize = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, NULL);
	//wchar_t 메모리 할당
	pStr = new WCHAR[strSize];
	//형 변환
	MultiByteToWideChar(CP_ACP, 0, str, strlen(str) + 1, pStr, strSize);
	return pStr;

}
char* ConvertWCtoC(const wchar_t* str)
{
	//반환할 char* 변수 선언
	char* pStr;
	//입력받은 wchar_t 변수의 길이를 구함
	int strSize = WideCharToMultiByte(CP_ACP, 0, str, -1, NULL, 0, NULL, NULL);
	//char* 메모리 할당
	pStr = new char[strSize];
	//형 변환
	WideCharToMultiByte(CP_ACP, 0, str, -1, pStr, strSize, 0, 0);
	return pStr;
}

std::string multibyte_to_utf8(const std::string& str) {
	int nLen = str.size();
	wchar_t warr[256];
	MultiByteToWideChar(CP_ACP, 0, (LPCSTR)str.c_str(), -1, warr, 256);
	char carr[256];
	memset(carr, '\0', sizeof(carr));
	WideCharToMultiByte(CP_UTF8, 0, warr, -1, carr, 256, NULL, NULL);
	return carr;
}


int main()
{
	SOCKET* targetSocket;
	Manager& m = Manager::getIncetance();
	m.ServerON("Data.json");

	fd_set read, tmp;
	TIMEVAL time;
	FD_ZERO(&read);
	FD_SET(m.ServerSocket, &read);
	int req;
	while (true)
	{
		tmp = read;
		time.tv_sec = 1;
		time.tv_usec = 0;
		req = select(NULL, &tmp, NULL, NULL, &time);
		if (SOCKET_ERROR == req)
		{
			cout << "req Error";
			break;
		}
		if (0 == req)
		{
			continue;
		}
		for (int i = 0; i < read.fd_count; i++)
		{
			targetSocket = &read.fd_array[i];
			if (!FD_ISSET(*targetSocket, &tmp))
			{
				continue;
			}
			if (m.ServerSocket == *targetSocket)
			{
				SOCKADDR_IN clientAddr;
				int size = sizeof(clientAddr);

				SOCKET cid = accept(m.ServerSocket, (SOCKADDR*)&clientAddr, &size);
				USER* user = new USER(cid, clientAddr);

				m.UserList.insert(make_pair(cid, user));
				FD_SET(cid, &read);
				user->SendMsg("안녕하세요. 텍스트 채팅 서버 입니다. 환영합니다.\r\n**로그인 명령어(LOGIN)를 사용해주세요. \r\n");
				printf("%s:%d connected\n", m.UserList[cid]->GetIP(), m.UserList[cid]->GetPort());
			}
			else
			{
				string msg;
				char tmp_msg[PACKET_SIZE] = { 0 };
				bool isConnect = false;
				do {
					ZeroMemory(&tmp_msg, PACKET_SIZE);
					if (recv(*targetSocket, tmp_msg, PACKET_SIZE, 0)==0)
					{
						//연결 종료
						printf("[%d] is Disconneted\n", *targetSocket);
						FD_CLR(*targetSocket, &read);
						m.DisConnect(&tmp.fd_array[i]);
						isConnect = false;
						break;
					}
					isConnect = true;
					msg += tmp_msg;
				} while (!(*(msg.end() - 2) == '\r' && *(msg.end() - 1) == '\n'));

				if (!isConnect) 
				{
					continue;
				}
				
				string dataBuffer = msg.substr(0, msg.length() - 2);
				m.Print(string(m.UserList[*targetSocket]->GetIP()) + ":" + to_string(m.UserList[*targetSocket]->GetPort()) + " [" + m.UserList[*targetSocket]->GetName() + "]" + "msg is :" + dataBuffer + "\r\n");

				vector<string> orderList = m.Split(dataBuffer, " ", 2);

				bool order = m.ExcuteOrder(m.UserList[*targetSocket], orderList);

				switch (m.UserList[*targetSocket]->GetState())
				{
				case EState::Auth:
					if (!order)
					{
						if (m.UserList[*targetSocket]->GetState() == EState::Auth)
						{
							m.UserList[*targetSocket]->SendMsg("**로그인 명령어(LOGIN)를 사용해주세요.\r\n");
						}
					}
					break;

				case EState::Lobby:
					if (m.UserList[*targetSocket]->GetState() == EState::Lobby)
					{
						//입력창 출력
						m.SendPrompt(m.UserList[*targetSocket]);
					}
					break;

				case EState::Room:
					//룸 전체에게 채팅 보내기
					if (!order) {
						for (USER* u : m.UserList[*targetSocket]->GetmyRoom()->GetUsers())
						{
							u->SendMsg(m.UserList[*targetSocket]->GetName() + ">" + dataBuffer + "\r\n");
						}
					}
					break;
				default:
					break;
				}
				//소켓 종료 요청 확인
				if (m.UserList[*targetSocket]->GetFin())
				{
					FD_CLR(*targetSocket, &read);
					m.DisConnect(&tmp.fd_array[i]);
					continue;
				}
			}
		}
	}
}