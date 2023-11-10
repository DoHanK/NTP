#pragma once
#include <array>
#include "Timer.h"
#include "Scene.h"
#include "Camera.h"
#include "Player.h"
#include "CUiManager.h"
///////////////서버///////////////////////////////
#include "Session.h"


#define NameBufferSize 7 //닉네임 6글자까지 입력 가능


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
	//DXGI 팩토리 인터페이스에 대한 포인터
	IDXGISwapChain3* m_pdxgiswapChain;
	//스왑 체인 인터페이스에 대한 포인터 이다. 주로 디스플레이를 제어하기 위하여 필요하다.
	ID3D12Device* m_pd3dDevice;
	//Direct3D 디바이스 인터페이스에 대한 포인터이다. 주로 리소스를 생성하기 위해 필요하다.

	bool m_bMsaa4xEnable = false;

	UINT m_nMsaa4xqualityLevels = 0;
	//msaa 다중 샘플링을 활성화하고 다중 샘플링 레벨을 설정한다.
	static const UINT m_nSwapChainBuffers = 2;
	//스왑체인의 후면 버퍼의 개수이다.
	UINT m_nSwapChainBufferIndex;
	//현재 스왑 체인의 후면 버퍼 인덱스이다.

	ID3D12Resource* m_ppd3dRenderTargetBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap* m_pd3dRtvDescriptorHeap;
	UINT m_nRtvDescriptorIncrementSize;
	//렌더 타겟 버퍼, 서술자 힙 인터페이스 포인터, 렌더 타겟 서술자 원소의 크기이다.

	ID3D12Resource* m_pd3dDepthStencilBuffer;
	ID3D12DescriptorHeap* m_pd3dDsvDescriptorHeap;
	UINT m_nDsvDescriptorIncrementSize;
	//깊이-스텐실 버퍼, 서술자 힙 인터페이스 포인터, 깊이-스텐실 서술자 원소의 크기이다. 
	ID3D12CommandQueue* m_pd3dCommandQueue;
	ID3D12CommandAllocator* m_pd3dCommandAllocator;
	ID3D12GraphicsCommandList* m_pd3dCommandList;
	//명령 큐, 명령 할당자, 명령 리스트 인터페이스 포인터이다.

	ID3D12PipelineState* m_pd3dPipelineState;
	//그래픽스 파이프라인 상태 객체에 대한 인터페이스 포인터이다.

	ID3D12Fence* m_pd3dFence;
	//6장에 배열이 추가되면서 삭제됨
	//UINT64 m_nFenceValue;

	HANDLE m_hFenceEvent;
	//펜스 인터페이스 포인터 ,펜스의 값, 이벤트 핸들이다.


	//다음은 게임 프레임 워크에서 사용할 타이머이다.
	CGameTimer m_GameTimer;
	//다음은 프레임레이트를 주 윈도우의 캡션에 출력하기 위한 문자열이다
	_TCHAR m_pszFrameRate[50];
	//후면 버퍼마다 현재의 펜스값을 관리하기 위하여 m_nFenceValue 멤버이다.
	UINT64 m_nFenceValues[m_nSwapChainBuffers];
	//씬을 그리기 위한 멤버변수
	CScene* m_pScene;

	CCamera* m_pCamera = NULL;

	CUiManager* m_pUIManager = NULL;
	CUiManager* m_pScoreManager = NULL; //score&NickName
	CMeshManager* m_pMeshManager = NULL;
	bool m_bRenderBoundingBox = false;
	//현재 게임 상태
	int m_GameState = InitStage;
	int m_PreGameState = InitStage;
	std::array<SESSION,MAX_USER> m_OtherPlayer;																									// 통신 받은 다른 플레이어
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
	//프레임워크를 초기화하는 함수이다.(주 윈도우가 생성되면 호출된다).
	void OnDestroy();

	void CreateSwapChain();
	void CreateRtvAndDsvDesriptorHeaps();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();
	//스왑 체인, 디바이스 ,서술자 힙 , 명령 큐/할당자/리스트를 생성하는 함수이다.

	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	//랜더 타겟 뷰와 깊이-스텐실 뷰를 생성하는 함수이다.

	void BuildObjects();
	void ReleaseObjects();
	//랜더링할 메쉬와 게임 객체를 생성하고 소멸하는 함수이다.

	//프레임워크의 핵심(사용자 입력, 애니메이션, 렌더링)을 구성하는 함수이다.
	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void WaitForGpuComplete();
	//CPU 와 GPU를 동기화하는 함수이다.

	//05장 따라하기 추가 changeSwapChainState()
	void ChangeSwapChainState();
	//06장 따라하기 추가
	void MoveToNextFrame();
	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM  lParam);
	bool OnProcessingUIMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void ProcessSelectedObject(DWORD dwDirection, float cxDelta, float cyDelta);
	//초기 버튼 생성
	void MakeInitButton();
	//게임 플레이 버튼 생성
	void MakePlayButton();
	//커스텀 생성 버튼
	void MakeCustomButton();
	//엔딩 생성 버튼
	void MakeEndButton();
	void ChangeScore();
	CD3DX12_GPU_DESCRIPTOR_HANDLE* GeneratorNumTex(int num);
	//서버 연동
	int InitSocket();
	
	void EnterRoom();


	void MakeLoginButton();
	void ShowInputName();
	void Makemulticustomebutton();
	void MakeReadyStage();
	void do_send(void* packet)																																// 데이터 송신
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
	//12장 
	//플레이어 객체에 대한 포인터이다.
	CTanker* m_pPlayer = NULL;
	//마지막으로 마우스 버튼을 클릭할 때의 마우스 커서의 위치다.
	POINT m_ptOldCursorPos;

	CGameObject* m_pSelectedObject = NULL;

	bool					m_ready = false;
	Color					m_color = blue;
	std::string				m_NickName = "seojin";								//PlayName

	bool	m_conneted = false;
	//Recv정보 처리 함수
	void process_packet(int c_id, char* packet);

	//print Player Info
	void PrintPlayerInfo(std::string s, int c_id);
	//레디상태 전송
	void SendReadyState();
};