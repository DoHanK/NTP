#include <DirectXMath.h>
#include <array>
using namespace DirectX;

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 1000;
constexpr int NAME_SIZE = 40;
constexpr int MAX_BULLETS = 30;

constexpr int MAX_USER = 3;

																																							// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_READY = 2;
constexpr char CS_ENTER_ROOM = 3;
constexpr char CS_BULLET = 4;
constexpr char CS_ATTACK = 5;
constexpr char CS_EXIT_ROOM = 6;
constexpr char CS_MINE_ATTACK = 7;

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_REMOVE_PLAYER = 4;
constexpr char SC_MOVE_PLAYER = 5;
constexpr char SC_READY = 6;
constexpr char SC_ENTER_ROOM = 7;
constexpr char SC_GAME_START = 8;
constexpr char SC_BULLET = 9;
constexpr char SC_REMOVE_BULLET = 10;
constexpr char SC_HITTED = 11;
constexpr char SC_RESULT = 12;
constexpr char SC_EXIT_ROOM = 13;



#pragma pack (push, 1)
struct CS_LOGIN_PACKET {																															// C -> S 로그인 패킷
	unsigned short	size;
	char			type;
	char			name[NAME_SIZE];
};		

																							 
struct CS_CHANGE_COLOR_PACKET {																														// C -> S 색깔 바꾸기 패킷
	unsigned short	size;
	char			type;
	int				color;
};

struct CS_MOVE_PACKET {																																// C -> S 이동 패킷
	unsigned short	size;
	char			type;
	XMFLOAT3		pos;
	XMFLOAT3		top_dir;
	XMFLOAT3		bottom_dir;
};

struct CS_BULLET_PACKET {																															// C -> S 총알 패킷
	unsigned short							size;
	char									type;
	std::array<XMFLOAT3, MAX_BULLETS>		bullets_pos;
	std::array<XMFLOAT3, MAX_BULLETS>		bullets_dir;
	std::array<bool, MAX_BULLETS>			in_use_bullets;
};

struct CS_READY_PACKET {																															// C -> S 레디 패킷
	unsigned short	size;
	char			type;
	bool			ready;
};

struct CS_ENTER_ROOM_PACKET {																														// C -> S 방 입장 패킷
	unsigned short	size;
	char			type;
	int				color;
};

struct CS_ATTACK_PACKET {
	unsigned short	size;
	char			type;
	int				id;
	int				bullet_index;
};

struct CS_MINE_ATTACK_PACKET {
	unsigned short	size;
	char			type;
	int				id;
};

struct CS_EXIT_ROOM_PACKET {
	unsigned short	size;
	char			type;
};

struct SC_LOGIN_INFO_PACKET {																														// S -> C 로그인 정보 패킷
	unsigned short	size;
	char			type;
	short			id;
	char			userName[NAME_SIZE];
	XMFLOAT3		pos;
	XMFLOAT3		top_dir;
	XMFLOAT3		bottom_dir;
};

struct SC_CHANGE_COLOR_PACKET {																														// S -> C 색깔 바꾸기 패킷
	unsigned short	size;
	char			type;
	short			id;
	int				color;
};

struct SC_ADD_PLAYER_PACKET {																														// S -> C 플레이어 추가 패킷
	unsigned short	size;
	char			type;
	short			id;
	int				hp;
	XMFLOAT3		pos;
	XMFLOAT3		top_dir;
	XMFLOAT3		bottom_dir;
	char			name[NAME_SIZE];
};

struct SC_REMOVE_PLAYER_PACKET {																													// S -> C 플레이어 제거 패킷
	unsigned short	size;
	char			type;
	short			id;
};

struct SC_MOVE_PLAYER_PACKET {																														// S -> C 이동 패킷
	unsigned short	size;
	char			type;
	short			id;
	XMFLOAT3		pos;
	XMFLOAT3		top_dir;
	XMFLOAT3		bottom_dir;
};

struct SC_HITTED_PACKET {																															// S -> C 피격 패킷
	unsigned short	size;	
	char			type;
	short			id;
	int				hp;
};

struct SC_BULLET_PACKET {																															// S -> C 총알 패킷
	unsigned short							size;
	char									type;
	short									id;
	std::array<XMFLOAT3, MAX_BULLETS>		bullets_pos;
	std::array<XMFLOAT3, MAX_BULLETS>		bullets_dir;
	std::array<bool, MAX_BULLETS>			in_use_bullets;
};

struct SC_READY_PACKET {
	unsigned short	size;
	char			type;
	short			id;
	bool			ready;
};

struct SC_ENTER_ROOM_PACKET {
	unsigned short	size;
	char			type;
	short			id;
	char			name[NAME_SIZE];
	int				color;
	int				pos_num;
	bool			ready;
};

struct SC_GAME_START_PACKET {
	unsigned short	size;
	char			type;
};

struct SC_RESULT_PACKET {
	unsigned short	size;
	char			type;
	short			id;
	int				rank;
};

struct SC_EXIT_ROOM_PACKET {
	unsigned short	size;
	char			type;
	short			id;
};

#pragma pack (pop)