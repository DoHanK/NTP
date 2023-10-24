#pragma once
#include "Mesh.h"
#include "Camera.h"
#include"d3dx12.h"
class CShader;

class CGameObject
{
public:
	CGameObject();
	virtual ~CGameObject();

private:
	int m_nReferences = 0;

public:

	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }
	char							m_pstrName[256] = { '\0' };

	bool						m_bActive = true;
	XMFLOAT3					m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	float						m_fMovingSpeed = 0.0f;
	float						m_fMovingRange = 0.0f;


	XMFLOAT4X4 m_xmf4x4World;
	XMFLOAT4X4* m_xmf4x4transformd;
	CSumMesh* m_pMesh = NULL;

	CBoundingBoxMesh*		m_pBoundingBoxMesh = NULL;
	
	CShader* m_pShader = NULL;
	//11�� �߰� �̵��� ȸ���� ���� ������
	
	BoundingOrientedBox  m_BoundingBox = BoundingOrientedBox();

	vector< pair<string,XMFLOAT4X4>> m_xmf4x4Animation;

	CD3DX12_GPU_DESCRIPTOR_HANDLE* m_TextureAddr=nullptr;
	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();


	//���� ��ü�� ���� x-��,y-��,z-�� �������� �̵��Ѵ�.

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void Move(XMFLOAT3& vDirection, float fSpeed);
	//���� ��ü�� ȸ��(x-�� ,y-�� , z-��)�Ѵ�.
	void Rotate(float fPitch = 10.0f, float fTaw = 10.0f, float fRoll = 10.0f);



public:
	//���� ��ġ ����
	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3 xmf3Position);
	void SetMovingDirection(const XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }
	void SetActive(bool bActive) { m_bActive = bActive; }
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	void SetMovingRange(float fRange) { m_fMovingRange = fRange; }

	void ReleaseUploadBuffers();
	virtual void SetMesh(CSumMesh* pMesh);
	virtual void SetShader(CShader* pShader);

	virtual void Animate(float fTimeElapsed);

	virtual void OnPrePareRender();
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	bool IsVisible(CCamera* pCamera = NULL);

	void UpdateAllTansform();
	void UpdateAllTansform(vector< pair<string, XMFLOAT4X4>> T);
	virtual void UpdateBoundingBox();
public: 
	//�� �޽����� ��� ���� ����
	void InitAnimaition();
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);

	//11�� �߰�
	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	//��� ������ ������ �����Ѵ�.
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void ReleaseShaderVariables();
public:
	//�� ��ǥ���� ��ŷ ������ �����Ѵ�. 
	void GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, XMFLOAT3* pxmf3PickRayOrigin, XMFLOAT3* pxmf3PickRayDirection);
	//ī�޶� ��ǥ���� �� ���� ���� �� ��ǥ���� ��ŷ ������ �����ϰ� ��ü���� ������ �˻��Ѵ�. 
	int PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfHitDistance );
	void RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Mine Object
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CMine : public CGameObject {
public:


public:

	virtual void Animate(float fTimeElapsed);


};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CMissile Object
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CMissile : public CGameObject {
public:
	CMissile(float fEffectiveRange);
	~CMissile() {};


	float						m_fBulletEffectiveRange = 50.0f;
	float						m_fMovingDistance = 0.0f;
	float						m_fRotationAngle = 0.0f;
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fElapsedTimeAfterFire = 0.0f;
	float						m_fLockingDelayTime = 0.3f;
	float						m_fLockingTime = 4.0f;
	CGameObject* m_pLockedObject = NULL;


	XMFLOAT3					m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	float						m_fRotationSpeed = 0.0f;

	void SetRotationAxis(const XMFLOAT3& xmf3RotationAxis) { m_xmf3RotationAxis = Vector3::Normalize(xmf3RotationAxis); }
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }
	void SetFirePosition(XMFLOAT3 xmf3FirePosition);
	void Reset();


public:
	virtual void Animate(float fTimeElapsed);

};


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//TankOject
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class CTank : public CGameObject {
public:
	CTank(CSumMesh* Mesh);
	~CTank() {};
	BoundingOrientedBox TopBoundingBox;
	XMFLOAT4X4* TopTransform;
	CSumMesh* TopMesh = nullptr;
	virtual void Animate(float fTimeElapsed);
	virtual void UpdateBoundingBox();

};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Billboard
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

class BillBoard : public CGameObject {
public:
	BillBoard() {};
	~BillBoard() {};
	CBillboardMesh* m_BillMesh;
	// Set
	float m_rows=8;
	float m_cols=8;
	
	float m_row=0;
	float m_col=0;

	float SettedTimer = 0.5f;
	float Timer = 0;
	void SetRowNCol(float row, float col) { m_rows = row; m_cols = col; }
	virtual void Animate(float fTimeElapsed);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);
	void Update(XMFLOAT3 xmf3Target, XMFLOAT3 xmf3Up);
};