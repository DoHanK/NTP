#include "Session.h"


void process_packet(int c_id, char* packet)								//패킷 처리함수
{
	cout << "process_packet called" << endl;
	switch (packet[1]) {
	case SC_LOGIN_INFO: {
		cout << "Recv Login Info Packet Server ! " << endl;
		SC_LOGIN_INFO_PACKET* p = reinterpret_cast<SC_LOGIN_INFO_PACKET*>(packet);
		cout << "Id = " << p->id << endl;
		cout << "Money = " << p->money << endl;
		break;
	}
	}
}

