#pragma once

#include "NetWork.h"
#include "stdafx.h"

using namespace std;

//char* SERVERIP = (char*)"127.0.0.1";
#define SERVERIP "127.0.0.1"

void process_packet(int c_id, char* packet);

class Status {
private:
	int            hp;
	int            speed;
	XMFLOAT3       pos;
	XMFLOAT3       topDir;
	XMFLOAT3       bottomDir;
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
	char			recvBuffer[BUF_SIZE];
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

	void do_recv()      // 데이터 수신
	{
		recvLen = ::recv(socket, recvBuffer, BUF_SIZE, 0);
		if (recvLen <= 0)
		{
			int errCode = ::WSAGetLastError();
			cout << "Bind ErrorCode : " << errCode << endl;
			error = true;
		}
		process_packet(id, recvBuffer);
	}

	void do_send(void* packet)      // 데이터 송신
	{
		recvLen = send(socket, recvBuffer, recvLen, 0);
		if (recvLen == SOCKET_ERROR) {
			int errCode = ::WSAGetLastError();
			cout << "Send ErrorCode : " << errCode << endl;
			error = true;
		}
	}
};



void process_packet(int c_id, char* packet);//패킷 처리 함수



