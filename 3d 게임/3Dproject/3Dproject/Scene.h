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
	//7�� �����ϱ� �߰�
	void ReleaseUploadBuffers();
	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);

	//�׷��� ��Ʈ �ñ׳��ĸ� �����Ѵ�.
	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice);
	ID3D12RootSignature* GetGraphicsRootSignature();

	//���� ��� ���� ��ü�鿡 ���� ���콺 ��ŷ�� �����Ѵ�. 



	ID3D12PipelineState* m_pd3dPipelineState = NULL;
	//���������� ���¸� ��Ÿ���� �������̽� �������̴�.


	void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
	//�޽� �Ŵ����� ���ؼ� �ߺ��� �޽����� �ϳ��� �޽��� ������.
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

	//�浹�Լ�
	//���� ��� ���� ��ü�鿡 ���� ���콺 ��ŷ�� �����Ѵ�. 
	CGameObject *PickObjectPointedByCursor(int xClient, int yClient, CCamera *pCamera);

	void RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera);

	void LoadSceneObjectsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, char* pstrFileName);
	
	bool PlayerToObject();

	void BulletToObject();

	//���� -> �ٸ� �÷��̾� ���� �־��ֱ�
	void InitOtherPlayer(std::array<SESSION, MAX_USER> &Players, int id);
	//���� ������Ʈ
	void UpdateOtherPlayer(std::array<SESSION, MAX_USER>& Players, int id);
	void ReomvePlayer(int id);
	void InitBullet(void* packet);
protected:
//��ġó���� ���� ���� ���̴����� ����Ʈ�� ǥ��
	CShader* m_pShaders = NULL;
	int m_nShaders = 0;


	void BulletToTank();




	ID3D12RootSignature* m_pd3dGraphicsRootSignature = NULL;
};

