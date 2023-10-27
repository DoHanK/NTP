#include <iostream>
#include <thread>
#include <vector>
#include <array>

#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>

#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

class Status {
private:
	int hp;
	int speed;
	XMFLOAT3 pos;
	XMFLOAT3 topDir;
	XMFLOAT3 bottomDir;
public:
	// setter
	void change_hp(int Hp) {
		hp = Hp;
	}
	void change_speed(int Speed) {
		speed = Speed;
	}
	void change_pos(XMFLOAT3 Pos) {
		pos = Pos;
	}
	void change_top_dir(XMFLOAT3 TopDir) {
		topDir = TopDir;
	}
	void change_bottom_dir(XMFLOAT3 BottomDir) {
		bottomDir = BottomDir;
	}

	// getter
	int get_hp() {
		return hp;
	}
	int get_speed() {
		return speed;
	}
	XMFLOAT3 get_pos() {
		return pos;
	}
	XMFLOAT3 get_top_dir() {
		return topDir;
	}
	XMFLOAT3 get_bottom_dir() {
		return bottomDir;
	}

};

class SESSION {
public:
	bool			in_use;
	int				id;
	SOCKET			socket;
	Status			status;
	int				money;
	std::string		userName;
	bool			ready;
	int				recvLen;
	char			recvBuffer[BUF_SIZE + 1];
	int				recvLen;
	bool			error;

public:
	SESSION() : socket(0), in_use(false)
	{
		id = -1;
		status.change_pos({ 0.f, 0.f, 0.f });
		status.change_top_dir({ 0.f, 0.f, 0.f });
		status.change_bottom_dir({ 0.f, 0.f, 0.f });
		ready = false;
	}

	~SESSION() {}

	void do_recv()		// 데이터 수신
	{
		recvLen = ::recv(socket, recvBuffer, BUF_SIZE, 0);
		if (recvLen <= 0)
		{
			int errCode = ::WSAGetLastError();
			cout << "Bind ErrorCode : " << errCode << endl;
			error = true;
		}
	}

	void do_send(void* packet)		// 데이터 송신
	{
		recvLen = send(socket, recvBuffer, recvLen, 0);
		if (recvLen == SOCKET_ERROR) {
			int errCode = ::WSAGetLastError();
			cout << "Send ErrorCode : " << errCode << endl;
			error = true;
		}
	}
	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.id = id;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		strcpy(p.userName, userName.c_str());
		p.money = money;

		do_send(&p);
	}


	void send_move_packet(int c_id);
	void send_attack_packet(int c_id);
	void send_dead_packet(int c_id);
	void send_hitted_packet(int c_id);
};

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i)
		if (clients[i].in_use == false)
			return i;
	return -1;
}

array<SESSION, MAX_USER> clients;


// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	int client_id = get_new_client_id();
	if (client_id != -1) {
		clients[client_id].in_use = true;
		clients[client_id].status.change_pos({ 0.f,0.f,0.f });
		clients[client_id].status.change_top_dir({ 0.f,0.f,0.f });
		clients[client_id].status.change_bottom_dir({ 0.f,0.f,0.f });


		clients[client_id].id = client_id;
		clients[client_id].ready = false;
		clients[client_id].socket = (SOCKET)arg;
		clients[client_id].do_recv();
		cout << "Client [" << client_id << "] Login" << endl;
	}
	else {
		cout << "Max user exceeded.\n";
	}

	char addr[INET_ADDRSTRLEN];
	int addrlen;
	char recvBuffer[BUF_SIZE + 1];

	// 클라이언트 정보 얻기
	addrlen = sizeof(clientaddr);
	getpeername(client_sock, (struct sockaddr*)&clientaddr, &addrlen);
	inet_ntop(AF_INET, &clientaddr.sin_addr, addr, sizeof(addr));

	while (1) {
		clients[client_id].do_recv();
	}

	// 소켓 닫기
	closesocket(client_sock);
	printf("[TCP 서버] 클라이언트 종료: IP 주소=%s, 포트 번호=%d\n",
		addr, ntohs(clientaddr.sin_port));
	return 0;
}

int main(int argc, char* argv[])
{
	// 윈속 초기화
	WSAData wsaData;
	if (::WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return 0;

	// 소켓 생성
	SOCKET listenSocket = ::socket(AF_INET, SOCK_STREAM, 0);
	if (listenSocket == INVALID_SOCKET)
	{
		int errCode = ::WSAGetLastError();
		cout << "Socket ErrorCode : " << errCode << endl;
		return 0;
	}

	// bind()

	SOCKADDR_IN serverAddr; // Ipv4;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY); // < 니가 알아서 해줘
	serverAddr.sin_port = ::htons(PORT_NUM);    // 80 : HTTP

	if (::bind(listenSocket, (SOCKADDR*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
	{
		int errCode = ::WSAGetLastError();
		cout << "Bind ErrorCode : " << errCode << endl;
		return 0;
	}

	// listen()
	if (::listen(listenSocket, SOMAXCONN) == SOCKET_ERROR)
	{
		int errCode = ::WSAGetLastError();
		cout << "Bind ErrorCode : " << errCode << endl;
		return 0;
	}

	// 데이터 통신에 사용할 변수
	SOCKET clientSocket;
	SOCKADDR_IN clientAddr;
	int addrLen;

	vector <thread> worker_threads;
	int num_threads = std::thread::hardware_concurrency();

	while (1) {
		// accept()
		addrLen = sizeof(clientAddr);
		clientSocket = accept(listenSocket, (SOCKADDR*)&clientAddr, &addrLen);
		if (clientSocket == INVALID_SOCKET)
		{
			int errCode = ::WSAGetLastError();
			cout << "Bind ErrorCode : " << errCode << endl;
			return 0;
		}

		// 손님 입장!
		char ipAddress[16];
		::inet_ntop(AF_INET, &clientAddr.sin_addr, ipAddress, sizeof(ipAddress));
		cout << "Client Connected! IP = " << ipAddress << endl;

		// 스레드 생성
		worker_threads.emplace_back(ProcessClient, (LPVOID)clientSocket);
	}
	for (auto& th : worker_threads)
		th.join();
	// 소켓 닫기
	closesocket(listenSocket);

	// 윈속 종료
	WSACleanup();
	return 0;
}
