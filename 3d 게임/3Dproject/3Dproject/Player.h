#pragma once

#define DIR_FORWARD 0x01
#define DIR_BACKWARD 0x02
#define DIR_LEFT 0x04
#define DIR_RIGHT 0x08
#define DIR_UP 0x10
#define DIR_DOWN 0x20

#define bottom_frame 30.f
#define BULLETS			50
#define MINES			2
#include "GameObject.h"
#include "Camera.h"
#include "Shader.h"
#include "NetWork.h"

class CPlayer :public CGameObject
{
protected:
	//플레이어의 위치 벡터 , x y z축 벡터이다.
	XMFLOAT3			m_xmf3Position;
	XMFLOAT3			m_xmf3Right;
	XMFLOAT3			m_xmf3Up;
	XMFLOAT3			m_xmf3Look;
	XMFLOAT3			m_xmf3BeforePosition;
	
	//플레이어가 로컬 x y z 축으로 얼마만큼 회전했는가를 나타낸다.
	float				m_fPitch;
	float				m_fYaw;
	float				m_fRoll;

	//플레이어의 이동 속도를 나타내는 벡터
	XMFLOAT3		m_xmf3Velocity;
	//플레이어에 작용하느나 중력을 나타내는 벡터이다.
	XMFLOAT3		m_xmf3Gravity;
	//xz평면에서 플레이어의 이동 속력의 최대값을 나타낸다.
	float			m_fMaxVelocityXZ;
	//y-축 방향으로 플레이어의 이동속력의 최대값을 나타낸다.
	float			m_fMaxVelocityY;
	//플ㄹ레이어의 마찰력을 나타냄
	float			m_fFriction;
	//플레이어의 위치가 바뀔때맘다 호출되는 onPlaterUpdateCallBack() 함수
	LPVOID			m_pPlayerUpdatedContext;

	//카메라의 위치가 바뀔때마다~~~ 함수
	LPVOID			m_pCameraUpdatedContext;

	//플레이어에 현재 설정된 카메라이다.
	CCamera* m_pCamera = NULL;

public:

	CPlayer();
	virtual ~CPlayer();

	XMFLOAT3& GetPosition() { return(m_xmf3Position); }
	XMFLOAT3& GetLookVector()  { return (m_xmf3Look); }
	XMFLOAT3& GetUpVector()  { return (m_xmf3Up); }
	XMFLOAT3& GetRightVector(){ return(m_xmf3Right); }
	XMFLOAT3& GetBeforePosition() { return(m_xmf3BeforePosition);}

	void SetPosition(XMFLOAT3& position) { m_xmf3Position = position; };
	void SetFriction(float fFriction) { m_fFriction = fFriction; }
	void SetGravity(XMFLOAT3& xmf3Gravity) { m_xmf3Gravity = xmf3Gravity; }
	void SetMaxVelocityXZ(float fMaxVelocity) { m_fMaxVelocityXZ = fMaxVelocity; }
	void SetMaxVelocityY(float fMaxVelocity) { m_fMaxVelocityY = fMaxVelocity; }
	void SetVelocity(XMFLOAT3& xmf3Velocity) { m_xmf3Velocity = xmf3Velocity; }
	void SetBeforePosition(XMFLOAT3& position) { m_xmf3BeforePosition = position; }
	//플레이어의 위치를  xmf3Position 위치로 설정하나 xmf3Position 벡터에서 현재 플레이어의 위치 벡터를 빼면 현재 플레이어의 위치에서 xmf3Posion 방향 벡터가 된다.

	void SetPositon(XMFLOAT3& xmf3Position) {
		Move(XMFLOAT3(xmf3Position.x -
			m_xmf3Position.x, xmf3Position.y - m_xmf3Position.y, xmf3Position.z - m_xmf3Position.z), false);
	}

	XMFLOAT3& GetVelocity() { return(m_xmf3Velocity); }
	float GetYaw() { return (m_fYaw); }
	float GetPitch() { return(m_fPitch); }
	float GetRoll() { return(m_fRoll); }

	CCamera* GetCamera() { return (m_pCamera); }
	void SetCamera(CCamera* pCamera) { m_pCamera = pCamera; }

	//플레이어 이동 함수
	void Move(ULONG nDirectoin, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);

	//플레이어의 회전 함수
	void Rotate(float x, float y, float z);

	//플레이어의 위치와 회전 정보를 경과시간에 따라 갱신하는 함수
	void  Update(float fTimeElapsed);

	//플레이어의 위치가 바뀔때마다 호출되는 함수와 그 함수에서 사용하는 정보를 설정하는 함수이다.

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) {}
	void SetPlayerUpdateContext(LPVOID pContect) { m_pCameraUpdatedContext= pContect; }
	virtual void OnCameraUpdateCallback(float fTimeElapsed) {}
	void SetCameraUpdateContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dcommandList);

	//카메라를 변경하기 위하여 호출하는 함수이다.
	CCamera* OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) {
		return (NULL);
	}

	//플레이어의 위치와 회전축으로부터 월드 변환 행렬을 생성하는 함수이다.
	virtual void OnPrepareRender();
	//플레이어의 카메라가 3인칭  카메라일때 플레이어를 렌더링 한다.
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

};

class Creater : public CPlayer
{
public:
	CIlluminatedShader* m_pShader = NULL;

	Creater(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature);
	virtual ~Creater() {};
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);
	virtual void OnPrepareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};

// CTankObject 
class CTanker : public CPlayer {
public:

	XMFLOAT4X4 TopTransform;
	XMFLOAT4X4 BottomTransform;
	float betweenangle;
	XMFLOAT3 CrossBetween;
	BoundingOrientedBox TopBoundingBox;

	CSumMesh* TopMesh;
	CSumMesh* GunMesh;
	CSumMesh* BottomMesh;
	CSumMesh* WheelMesh[4];

	CIlluminatedShader* m_pShader = NULL;
	XMFLOAT4X4* Top;
	XMFLOAT4X4* Bottom;
	XMFLOAT4X4* Gun;
	XMFLOAT4X4* wheel[4];
	//animaition 관련된 함수
	bool wheelanimation = false;
	bool flag_move_screen = 0;
	int BottomAnimationFrame = 0;
	CMissile* m_ppBullets[BULLETS];
	CMine* m_pMine[MINES];
	int UseMine = 0;
	float						m_fBulletEffectiveRange = 150.0f;
	float						m_CameraAngle = 0;
	bool						m_BCameraRotate = false;
	

	
	int m_Score = 0;

	bool m_BCameraMove = false;
	float m_CameraDistance = 2;



	CTanker(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList,
		ID3D12RootSignature* pd3dGraphicsRootSignature, CSumMesh* bulletMesh, 
		CD3DX12_GPU_DESCRIPTOR_HANDLE* TextureAddr, CSumMesh* MineMesh);
	virtual ~CTanker() {};
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed);

	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	virtual void  OnPrepareRender();
	virtual void UpdateAllTansform();
	void UpdateWheel();
	void FindFrameSet();
	float VectorSize(XMFLOAT3 V);
	virtual void Animate(float fTimeEelapsed);

	//미사일 관련
	void FireMissile();
	void SetMine();
	virtual void UpdateBoundingBox();

	void ChangeColor(CD3DX12_GPU_DESCRIPTOR_HANDLE* TankTexAddr,CD3DX12_GPU_DESCRIPTOR_HANDLE* ObjectTexAddr);
	void initGame(void* packet);
};