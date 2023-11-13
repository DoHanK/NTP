#define _CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <thread>
#include <vector>
#include <array>
#include <mutex>

#include <WinSock2.h>
#include <MSWSock.h>
#include <ws2tcpip.h>

#include "protocol.h"

#pragma comment(lib, "WS2_32.lib")
#pragma comment(lib, "MSWSock.lib")

using namespace std;

void process_packet(int c_id, char* packet);


array<int, 3> room{ -1,-1,-1 };


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
	int				color;
	int				money;
	std::string		userName;
	bool			ready;
	char			recvBuffer[BUF_SIZE ];
	int				recvLen;
	char			sendBuffer[BUF_SIZE];
	int				sendLen;
	bool			error;
	int				pos_num;
	mutex m;
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

	void do_recv()																																			// 데이터 수신
	{
		recvLen = ::recv(socket, recvBuffer, BUF_SIZE, MSG_WAITALL);
		if (recvLen <= 0)
		{
			int errCode = ::WSAGetLastError();
			cout << "Bind ErrorCode : " << errCode << endl;
			error = true;
			room[pos_num] = -1;
			return;
		}
		process_packet(id, recvBuffer);
	}

	void do_send(void* packet)																																// 데이터 송신
	{
		sendLen = int(reinterpret_cast<char*>(packet)[0]);
		memcpy(sendBuffer, reinterpret_cast<char*>(packet),sendLen);
		sendLen = send(socket, sendBuffer, sendLen, 0);
		if (sendLen == SOCKET_ERROR) {
			int errCode = ::WSAGetLastError();
			cout << "Send ErrorCode : " << errCode << endl;
			error = true;
		}
	}
	
	void send_login_info_packet()
	{
		SC_LOGIN_INFO_PACKET p;
		p.size = sizeof(SC_LOGIN_INFO_PACKET);
		p.type = SC_LOGIN_INFO;
		p.id = id;
		p.money = money;
		strcpy(p.userName, userName.c_str());
		p.pos = status.get_pos();
		p.top_dir = status.get_top_dir();
		p.bottom_dir = status.get_bottom_dir();
		do_send(&p);
	}
	void send_ready_packet(int c_id);
	void send_enter_room_packet(int c_id);
	void send_move_packet(int c_id);
	void send_attack_packet(int c_id);
	void send_dead_packet(int c_id);
	void send_hitted_packet(int c_id);
};

array<SESSION, MAX_USER> clients;																												// 클라이언트 배열 생성

void SESSION::send_ready_packet(int c_id)
{
	SC_READY_PACKET p;
	p.size = sizeof(SC_READY_PACKET);
	p.type = SC_READY;
	p.id = c_id;
	p.ready = clients[c_id].ready;
	do_send(&p);
}
void SESSION::send_enter_room_packet(int c_id)
{
	SC_ENTER_ROOM_PACKET p;
	p.size = sizeof(SC_ENTER_ROOM_PACKET);
	p.type = SC_ENTER_ROOM;
	p.id = c_id;
	strcpy(p.name, clients[c_id].userName.c_str());
	p.color = clients[c_id].color;
	p.pos_num = clients[c_id].pos_num;
	do_send(&p);
}


void process_packet(int c_id, char* packet)
{
	cout << "process_packet called" << endl;
	switch (packet[1]) {
	case CS_LOGIN: {
		cout << "Recv Login Packet From Client Num : " << c_id << endl;
		CS_LOGIN_PACKET* p = reinterpret_cast<CS_LOGIN_PACKET*>(packet);
		clients[c_id].userName = p->name;
		clients[c_id].send_login_info_packet();
		std::cout << "User Name - " << clients[c_id].userName << endl;
		cout << "Send Login Info Packet To Client Num : " << c_id << endl;
		break;
	}
	case CS_READY: {
		CS_READY_PACKET* p = reinterpret_cast<CS_READY_PACKET*>(packet);
		if (clients[c_id].ready)
			clients[c_id].ready = false;
		else
			clients[c_id].ready = true;

		for (auto &pl : clients) {
			if (pl.in_use == false)
				continue;
			pl.send_ready_packet(c_id);
		}
		break;
	}
	case CS_ENTER_ROOM: {
		cout << "Recv Enter Room Packet From Client Num : " << c_id << endl;

		CS_ENTER_ROOM_PACKET* p = reinterpret_cast<CS_ENTER_ROOM_PACKET*>(packet);
		for (int i = 0; i < room.size(); ++i) {
			if (room[i] == -1) {
				room[i] = c_id;
				clients[c_id].pos_num = i;
				break;
			}
		}
		clients[c_id].color = p->color;
		
		for (auto &pl : clients) {
			if (pl.in_use == false)
				continue;
			if (pl.id == c_id)
				continue;
			pl.send_enter_room_packet(c_id);
		}
		for (int i = 0; i < room.size();  ++i) {
			if (room[i] != -1)
				clients[c_id].send_enter_room_packet(room[i]);
		}
		break;
	}
	}
}

int get_new_client_id()
{
	for (int i = 0; i < MAX_USER; ++i)
		if (clients[i].in_use == false)
			return i;
	return -1;
}



																																				// 클라이언트와 데이터 통신
DWORD WINAPI ProcessClient(LPVOID arg)
{
	int client_id = get_new_client_id();
	if (client_id != -1) {
		clients[client_id].in_use = true;
		clients[client_id].money = 123;
		clients[client_id].status.change_pos({ 0.f,0.f,0.f });
		clients[client_id].status.change_top_dir({ 0.f,0.f,0.f });
		clients[client_id].status.change_bottom_dir({ 0.f,0.f,0.f });


		clients[client_id].id = client_id;
		clients[client_id].ready = false;
		clients[client_id].error = false;
		clients[client_id].socket = (SOCKET)arg;
		cout << "Client [" << client_id << "] Login" << endl;
	}
	else {
		cout << "Max user exceeded.\n";
	}

	while (1) {
		clients[client_id].do_recv();
		if (clients[client_id].error) {
			break;
		}
	}

																																						// 소켓 닫기
	closesocket(clients[client_id].socket);
	clients[client_id].in_use = false;
	cout << clients[client_id].id << " 번 클라이언트 종료"<<endl;
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

	SOCKADDR_IN serverAddr;
	::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = ::htonl(INADDR_ANY);
	serverAddr.sin_port = ::htons(PORT_NUM);

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
