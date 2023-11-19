#include <DirectXMath.h>
using namespace DirectX;

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 40;

constexpr int MAX_USER = 3;

																																							// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;
constexpr char CS_READY = 2;
constexpr char CS_ENTER_ROOM = 3;

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_REMOVE_PLAYER = 4;
constexpr char SC_MOVE_PLAYER = 5;
constexpr char SC_READY = 6;
constexpr char SC_ENTER_ROOM = 7;
constexpr char SC_GAME_START = 8;

#pragma pack (push, 1)
struct CS_LOGIN_PACKET {																															// C -> S 로그인 패킷
	unsigned char	size;
	char			type;
	char			name[NAME_SIZE];
};		

																							 
struct CS_CHANGE_COLOR_PACKET {																														// C -> S 색깔 바꾸기 패킷
	unsigned char	size;
	char			type;
	int				color;
};

struct CS_MOVE_PACKET {																																// C -> S 이동 패킷
	unsigned char	ize;
	char			type;
	XMFLOAT3		pos;
	XMFLOAT3		top_dir;
	XMFLOAT3		bottom_dir;
};

struct CS_BULLET_PACKET {																															// C -> S 총알 패킷
	unsigned char	size;
	char			type;
	int				color;
	int				index;
	int				damage;
};

struct CS_READY_PACKET {																															// C -> S 레디 패킷
	unsigned char	size;
	char			type;
	bool			ready;
};

struct CS_ENTER_ROOM_PACKET {																														// C -> S 방 입장 패킷
	unsigned char	size;
	char			type;
	int				color;
};

struct SC_LOGIN_INFO_PACKET {																														// S -> C 로그인 정보 패킷
	unsigned char	size;
	char			type;
	short			id;
	int				money;
	char			 userName[NAME_SIZE];
	XMFLOAT3		pos;
	XMFLOAT3		top_dir;
	XMFLOAT3		bottom_dir;
};

struct SC_CHANGE_COLOR_PACKET {																														// S -> C 색깔 바꾸기 패킷
	unsigned char	size;
	char			type;
	short			id;
	int				color;
};

struct SC_ADD_PLAYER_PACKET {																														// S -> C 플레이어 추가 패킷
	unsigned char	size;
	char			type;
	short			id;
	int				hp;
	XMFLOAT3		pos;
	XMFLOAT3		top_dir;
	XMFLOAT3		bottom_dir;
	char			name[NAME_SIZE];
};

struct SC_REMOVE_PLAYER_PACKET {																													// S -> C 플레이어 제거 패킷
	unsigned char	size;
	char			type;
	short			id;
};

struct SC_MOVE_PLAYER_PACKET {																														// S -> C 이동 패킷
	unsigned char	size;
	char			type;
	short			id;
	XMFLOAT3		pos;
	XMFLOAT3		top_dir;
	XMFLOAT3		bottom_dir;
};

struct SC_HITTED_PACKET {																															// S -> C 피격 패킷
	unsigned char	size;	
	char			type;
	short			id;
	int				hp;
};

struct SC_BULLET_PACKET {																															// S -> C 총알 패킷
	unsigned char	size;
	char			type;
	int				index;
	int				color;
	XMFLOAT3		pos;
	XMFLOAT3		dir;
	bool			in_use;
};

struct SC_READY_PACKET {
	unsigned char	size;
	char			type;
	short			id;
	bool			ready;
};

struct SC_ENTER_ROOM_PACKET {
	unsigned char	size;
	char			type;
	short			id;
	char			name[NAME_SIZE];
	int				color;
	int				pos_num;
};

struct SC_GAME_START_PACKET {
	unsigned char	size;
	char			type;
};

#pragma pack (pop)