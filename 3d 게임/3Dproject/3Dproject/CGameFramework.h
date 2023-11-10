#pragma once
#include <array>
#include "Timer.h"
#include "Scene.h"
#include "Camera.h"
#include "Player.h"
#include "CUiManager.h"
///////////////����///////////////////////////////
#include "Session.h"


#define NameBufferSize 7 //�г��� 6���ڱ��� �Է� ����


enum Color {
	blue, red, green, yellow
};



class CGameFrameWork {
private:
	HINSTANCE m_hInstance;
	HWND m_hWnd;

	int m_nWndClientWidth;
	int m_nWndClientHeight;

	IDXGIFactory4* m_pdxgiFactory;
	//DXGI ���丮 �������̽��� ���� ������
	IDXGISwapChain3* m_pdxgiswapChain;
	//���� ü�� �������̽��� ���� ������ �̴�. �ַ� ���÷��̸� �����ϱ� ���Ͽ� �ʿ��ϴ�.
	ID3D12Device* m_pd3dDevice;
	//Direct3D ����̽� �������̽��� ���� �������̴�. �ַ� ���ҽ��� �����ϱ� ���� �ʿ��ϴ�.

	bool m_bMsaa4xEnable = false;

	UINT m_nMsaa4xqualityLevels = 0;
	//msaa ���� ���ø��� Ȱ��ȭ�ϰ� ���� ���ø� ������ �����Ѵ�.
	static const UINT m_nSwapChainBuffers = 2;
	//����ü���� �ĸ� ������ �����̴�.
	UINT m_nSwapChainBufferIndex;
	//���� ���� ü���� �ĸ� ���� �ε����̴�.

	ID3D12Resource* m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;
	UINT m_nRtvDescriptorIncrementSize;
	//���� Ÿ�� ����, ������ �� �������̽� ������, ���� Ÿ�� ������ ������ ũ���̴�.

	ID3D12Resource* m_pd3dDepthStencilBuffer;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;
	UINT m_nDsvDescriptorIncrementSize;
	//����-���ٽ� ����, ������ �� �������̽� ������, ����-���ٽ� ������ ������ ũ���̴�. 
	ID3D12CommandQueue* m_pd3dCommandQueue;
	ID3D12CommandAllocator* m_pd3dCommandAllocator;
	ID3D12GraphicsCommandList* m_pd3dCommandList;
	//��� ť, ��� �Ҵ���, ��� ����Ʈ �������̽� �������̴�.

	ID3D12PipelineState* m_pd3dPipelineState;
	//�׷��Ƚ� ���������� ���� ��ü�� ���� �������̽� �������̴�.

	ID3D12Fence* m_pd3dFence;
	//6�忡 �迭�� �߰��Ǹ鼭 ������
	//UINT64 m_nFenceValue;

	HANDLE m_hFenceEvent;
	//�潺 �������̽� ������ ,�潺�� ��, �̺�Ʈ �ڵ��̴�.


	//������ ���� ������ ��ũ���� ����� Ÿ�̸��̴�.
	CGameTimer m_GameTimer;
	//������ �����ӷ���Ʈ�� �� �������� ĸ�ǿ� ����ϱ� ���� ���ڿ��̴�
	_TCHAR m_pszFrameRate[50];
	//�ĸ� ���۸��� ������ �潺���� �����ϱ� ���Ͽ� m_nFenceValue ����̴�.
	UINT64 m_nFenceValues[m_nSwapChainBuffers];
	//���� �׸��� ���� �������
	CScene* m_pScene;

	CCamera* m_pCamera = NULL;

	CUiManager* m_pUIManager = NULL;
	CUiManager* m_pScoreManager = NULL; //score&NickName
	CMeshManager* m_pMeshManager = NULL;
	bool m_bRenderBoundingBox = false;
	//���� ���� ����
	int m_GameState = InitStage;
	int m_PreGameState = InitStage;
	std::array<SESSION,MAX_USER> m_OtherPlayer;																									// ��� ���� �ٸ� �÷��̾�
	SOCKET m_ServerSocket;
	int		m_myid;
	char	m_SendBuffer[BUF_SIZE];																												// SendBuffer
	int		sendLen;																													
	char	m_RecvBuffer[BUF_SIZE];																												// RecvBuffer
	int		recvLen;
	char	KeyInputBuffer[NameBufferSize];
public:
	CGameFrameWork();
	~CGameFrameWork();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	//�����ӿ�ũ�� �ʱ�ȭ�ϴ� �Լ��̴�.(�� �����찡 �����Ǹ� ȣ��ȴ�).
	void OnDestroy();

	void CreateSwapChain();
	void CreateRtvAndDsvDesriptorHeaps();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();
	//���� ü��, ����̽� ,������ �� , ��� ť/�Ҵ���/����Ʈ�� �����ϴ� �Լ��̴�.

	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	//���� Ÿ�� ��� ����-���ٽ� �並 �����ϴ� �Լ��̴�.

	void BuildObjects();
	void ReleaseObjects();
	//�������� �޽��� ���� ��ü�� �����ϰ� �Ҹ��ϴ� �Լ��̴�.

	//�����ӿ�ũ�� �ٽ�(����� �Է�, �ִϸ��̼�, ������)�� �����ϴ� �Լ��̴�.
	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void WaitForGpuComplete();
	//CPU �� GPU�� ����ȭ�ϴ� �Լ��̴�.

	//05�� �����ϱ� �߰� changeSwapChainState()
	void ChangeSwapChainState();
	//06�� �����ϱ� �߰�
	void MoveToNextFrame();
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM  lParam);
	bool OnProcessingUIMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void ProcessSelectedObject(DWORD dwDirection, float cxDelta, float cyDelta);
	//�ʱ� ��ư ����
	void MakeInitButton();
	//���� �÷��� ��ư ����
	void MakePlayButton();
	//Ŀ���� ���� ��ư
	void MakeCustomButton();
	//���� ���� ��ư
	void MakeEndButton();
	void ChangeScore();
	CD3DX12_GPU_DESCRIPTOR_HANDLE* GeneratorNumTex(int num);
	//���� ����
	int InitSocket();
	
	void EnterRoom();


	void MakeLoginButton();
	void ShowInputName();
	void Makemulticustomebutton();
	void MakeReadyStage();
	void do_send(void* packet)																																// ������ �۽�
	{
		sendLen = int(reinterpret_cast<char*>(packet)[0]);
		memcpy(m_SendBuffer, reinterpret_cast<char*>(packet), sendLen);
		sendLen = send(m_ServerSocket, m_SendBuffer, sendLen, 0);
		if (sendLen == SOCKET_ERROR) {
			int errCode = ::WSAGetLastError();
			cout << "Send ErrorCode : " << errCode << endl;
		}
	}
public:
	//12�� 
	//�÷��̾� ��ü�� ���� �������̴�.
	CTanker* m_pPlayer = NULL;
	//���������� ���콺 ��ư�� Ŭ���� ���� ���콺 Ŀ���� ��ġ��.
	POINT m_ptOldCursorPos;

	CGameObject* m_pSelectedObject = NULL;

	bool					m_ready = false;
	Color					m_color = blue;
	std::string				m_NickName = "seojin";								//PlayName

	bool	m_conneted = false;
	//Recv���� ó�� �Լ�
	void process_packet(int c_id, char* packet);

	//print Player Info
	void PrintPlayerInfo(std::string s, int c_id);
	//������� ����
	void SendReadyState();
};