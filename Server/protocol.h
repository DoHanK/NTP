#include <DirectXMath.h>
using namespace DirectX;

constexpr int PORT_NUM = 4000;
constexpr int BUF_SIZE = 200;
constexpr int NAME_SIZE = 20;

constexpr int MAX_USER = 4;

// Packet ID
constexpr char CS_LOGIN = 0;
constexpr char CS_MOVE = 1;

constexpr char SC_LOGIN_INFO = 2;
constexpr char SC_ADD_PLAYER = 3;
constexpr char SC_REMOVE_PLAYER = 4;
constexpr char SC_MOVE_PLAYER = 5;

#pragma pack (push, 1)
struct CS_LOGIN_PACKET {
	unsigned char	size;
	char			type;
	char			name[NAME_SIZE];
};

struct CS_CHANGE_COLOR_PACKET {
	unsigned char	size;
	char			type;
	int				color;
};

struct CS_MOVE_PACKET {
	unsigned char	ize;
	char			type;
	XMFLOAT3		pos;
	XMFLOAT3		top_dir;
	XMFLOAT3		bottom_dir;
};

struct CS_BULLET_PACKET {
	unsigned char	size;
	char			type;
	int				color;
	int				index;
	int				damage;
};

struct SC_LOGIN_INFO_PACKET {
	unsigned char	size;
	char			type;
	short			id;
	int				money;
	char			 userName[NAME_SIZE];
	XMFLOAT3		pos;
	XMFLOAT3		top_dir;
	XMFLOAT3		bottom_dir;
};

struct SC_CHANGE_COLOR_PACKET {
	unsigned char	size;
	char			type;
	short			id;
	int				color;
};

struct SC_ADD_PLAYER_PACKET {
	unsigned char	size;
	char			type;
	short			id;
	XMFLOAT3		pos;
	XMFLOAT3		top_dir;
	XMFLOAT3		bottom_dir;
	char			name[NAME_SIZE];
};

struct SC_REMOVE_PLAYER_PACKET {
	unsigned char	size;
	char			type;
	short			id;
};

struct SC_MOVE_PLAYER_PACKET {
	unsigned char	size;
	char			type;
	short			id;
	XMFLOAT3		pos;
	XMFLOAT3		top_dir;
	XMFLOAT3		bottom_dir;
};

struct SC_HITTED_PACKET {
	unsigned char	size;
	char			type;
	short			id;
	int				hp;
};

struct SC_BULLET_PACKET {
	unsigned char	size;
	char			type;
	int				index;
	int				color;
	XMFLOAT3		pos;
	XMFLOAT3		dir;
	bool			in_use;
};


#pragma pack (pop)