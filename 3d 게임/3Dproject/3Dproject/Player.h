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
	//�÷��̾��� ��ġ ���� , x y z�� �����̴�.
	XMFLOAT3			m_xmf3Position;
	XMFLOAT3			m_xmf3Right;
	XMFLOAT3			m_xmf3Up;
	XMFLOAT3			m_xmf3Look;
	XMFLOAT3			m_xmf3BeforePosition;
	
	//�÷��̾ ���� x y z ������ �󸶸�ŭ ȸ���ߴ°��� ��Ÿ����.
	float				m_fPitch;
	float				m_fYaw;
	float				m_fRoll;

	//�÷��̾��� �̵� �ӵ��� ��Ÿ���� ����
	XMFLOAT3		m_xmf3Velocity;
	//�÷��̾ �ۿ��ϴ��� �߷��� ��Ÿ���� �����̴�.
	XMFLOAT3		m_xmf3Gravity;
	//xz��鿡�� �÷��̾��� �̵� �ӷ��� �ִ밪�� ��Ÿ����.
	float			m_fMaxVelocityXZ;
	//y-�� �������� �÷��̾��� �̵��ӷ��� �ִ밪�� ��Ÿ����.
	float			m_fMaxVelocityY;
	//�ä����̾��� �������� ��Ÿ��
	float			m_fFriction;
	//�÷��̾��� ��ġ�� �ٲ𶧸��� ȣ��Ǵ� onPlaterUpdateCallBack() �Լ�
	LPVOID			m_pPlayerUpdatedContext;

	//ī�޶��� ��ġ�� �ٲ𶧸���~~~ �Լ�
	LPVOID			m_pCameraUpdatedContext;

	//�÷��̾ ���� ������ ī�޶��̴�.
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
	//�÷��̾��� ��ġ��  xmf3Position ��ġ�� �����ϳ� xmf3Position ���Ϳ��� ���� �÷��̾��� ��ġ ���͸� ���� ���� �÷��̾��� ��ġ���� xmf3Posion ���� ���Ͱ� �ȴ�.

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

	//�÷��̾� �̵� �Լ�
	void Move(ULONG nDirectoin, float fDistance, bool bVelocity = false);
	void Move(const XMFLOAT3& xmf3Shift, bool bVelocity = false);
	void Move(float fxOffset = 0.0f, float fyOffset = 0.0f, float fzOffset = 0.0f);

	//�÷��̾��� ȸ�� �Լ�
	void Rotate(float x, float y, float z);

	//�÷��̾��� ��ġ�� ȸ�� ������ ����ð��� ���� �����ϴ� �Լ�
	void  Update(float fTimeElapsed);

	//�÷��̾��� ��ġ�� �ٲ𶧸��� ȣ��Ǵ� �Լ��� �� �Լ����� ����ϴ� ������ �����ϴ� �Լ��̴�.

	virtual void OnPlayerUpdateCallback(float fTimeElapsed) {}
	void SetPlayerUpdateContext(LPVOID pContect) { m_pCameraUpdatedContext= pContect; }
	virtual void OnCameraUpdateCallback(float fTimeElapsed) {}
	void SetCameraUpdateContext(LPVOID pContext) { m_pCameraUpdatedContext = pContext; }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dcommandList);

	//ī�޶� �����ϱ� ���Ͽ� ȣ���ϴ� �Լ��̴�.
	CCamera* OnChangeCamera(DWORD nNewCameraMode, DWORD nCurrentCameraMode);
	virtual CCamera* ChangeCamera(DWORD nNewCameraMode, float fTimeElapsed) {
		return (NULL);
	}

	//�÷��̾��� ��ġ�� ȸ�������κ��� ���� ��ȯ ����� �����ϴ� �Լ��̴�.
	virtual void OnPrepareRender();
	//�÷��̾��� ī�޶� 3��Ī  ī�޶��϶� �÷��̾ ������ �Ѵ�.
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
	//animaition ���õ� �Լ�
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

	//�̻��� ����
	void FireMissile();
	void SetMine();
	virtual void UpdateBoundingBox();

	void ChangeColor(CD3DX12_GPU_DESCRIPTOR_HANDLE* TankTexAddr,CD3DX12_GPU_DESCRIPTOR_HANDLE* ObjectTexAddr);
	void initGame(void* packet);
};