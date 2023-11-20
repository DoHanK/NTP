#pragma once
#include "Session.h"
#include "Timer.h"
#include "Shader.h"
#include "Camera.h"
#include "CMeshManager.h"
#include "CUiManager.h"
#include "Player.h"


#define MAX_LIGHTS			16 

#define POINT_LIGHT			1
#define SPOT_LIGHT			2
#define DIRECTIONAL_LIGHT	3



struct LIGHT
{
	XMFLOAT4				m_xmf4Ambient;
	XMFLOAT4				m_xmf4Diffuse;
	XMFLOAT4				m_xmf4Specular;
	XMFLOAT3				m_xmf3Position;
	float 					m_fFalloff;
	XMFLOAT3				m_xmf3Direction;
	float 					m_fTheta; //cos(m_fTheta)
	XMFLOAT3				m_xmf3Attenuation;
	float					m_fPhi; //cos(m_fPhi)
	bool					m_bEnable;
	int						m_nType;
	float					m_fRange;
	float					padding;
};

struct LIGHTS
{
	LIGHT					m_pLights[MAX_LIGHTS];
	XMFLOAT4				m_xmf4GlobalAmbient;
	int						m_nLights;
};


class CScene
{public:

	CScene();
	~CScene() {};
	
	bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void CreateGraphicsPipelineState(ID3D12Device* pd3dDevice);

	void BuildObjects(ID3D12Device* pd3dDevice ,ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseObjects();

	bool ProcessInput();
	void AnimateObjects(float fTimeElapsed);
	void PrepareRender(ID3D12GraphicsCommandList* pd3dCommandList);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamrea);
	void BuildDefaultLightsAndMaterials();
	//7장 따라하기 추가
	void ReleaseUploadBuffers();
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	//그래픽 루트 시그너쳐를 생성한다.
	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature();

	//씬의 모든 게임 객체들에 대한 마우스 픽킹을 수행한다. 



	ID3D12PipelineState* m_pd3dPipelineState = NULL;
	//파이프라인 상태를 나타내는 인터페이스 포인터이다.


	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	//메쉬 매니저를 통해서 중복된 메쉬없이 하나의 메쉬를 공유함.
	CMeshManager* m_MeshManager;
	CUiManager* m_pUIManger;

	vector<CGameObject*> CGameObjects; 
	vector<CGameObject*> m_CGameBackGround;

	vector<CTank*> CTankObjects;
	CIlluminatedShader* m_pCllluminatedShader = NULL;
	CBoundingBoxShader* m_pBoundingBoxShader = NULL; 

	CTanker* m_pPlayer = NULL;
	vector<BillBoard*> m_BillBoardList;
	vector<BillBoard*> m_SubBillBoardList;
	LIGHT* m_pLights = NULL;
	int							m_nLights = 0;

	XMFLOAT4					m_xmf4GlobalAmbient;

	ID3D12Resource* m_pd3dcbLights = NULL;
	LIGHTS* m_pcbMappedLights = NULL;
	
	CMissile AllBullets[MAX_USER][BULLETS];

	//충돌함수
	//씬의 모든 게임 객체들에 대한 마우스 픽킹을 수행한다. 
	CGameObject *PickObjectPointedByCursor(int xClient, int yClient, CCamera *pCamera);

	void RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void LoadSceneObjectsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, char* pstrFileName);
	
	bool PlayerToObject();

	void BulletToObject();

	//레디 -> 다른 플레이어 정보 넣어주기
	void InitOtherPlayer(std::array<SESSION, MAX_USER> &Players, int id);
	//게임 업데이트
	void UpdateOtherPlayer(std::array<SESSION, MAX_USER>& Players, int id);
	void ReomvePlayer(int id);
	void InitBullet(void* packet);
protected:
//배치처리를 위한 씬을 세이더들의 리스트로 표현
	CShader* m_pShaders = NULL;
	int m_nShaders = 0;


	void BulletToTank();




	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
};

