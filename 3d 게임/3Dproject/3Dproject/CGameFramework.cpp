#include "stdafx.h"
#include "CGameFramework.h"	
#include "CMeshManager.h"
#include "NetWork.h"

#define _SERVER_TEST



CGameFrameWork::CGameFrameWork() {

	m_pdxgiFactory = NULL;
	m_pdxgiswapChain = NULL;
	m_pd3dDevice = NULL;

	m_pd3dCommandAllocator = NULL;
	m_pd3dCommandQueue = NULL;
	m_pd3dPipelineState = NULL;
	m_pd3dCommandList = NULL;

	for (int i = 0; i < m_nSwapChainBuffers; i++) m_ppd3dRenderTargetBuffers[i] = NULL;

	//06�忡�� �߰��� ����
	for (int i = 0; i < m_nSwapChainBuffers; i++) m_nFenceValues[i] = 0;
	m_pScene = NULL;
	

	m_pd3dRtvDescriptorHeap = NULL;
	m_nRtvDescriptorIncrementSize = 0;

	m_pd3dDepthStencilBuffer = NULL;
	m_pd3dRtvDescriptorHeap = NULL;
	m_nDsvDescriptorIncrementSize = 0;

	m_nSwapChainBufferIndex = 0;

	m_hFenceEvent = NULL;
	m_pd3dFence = NULL;
	for (UINT i = 0; i < m_nSwapChainBufferIndex; ++i) m_nFenceValues[i] = 0;

	m_nWndClientWidth = FRAME_BUFFER_WIDTH;
	m_nWndClientHeight = FRAME_BUFFER_HEIGHT;

	//�ð� �߰�
	_tcscpy_s(m_pszFrameRate, _T("LapaProject("));

	memset(KeyInputBuffer, '\0', NameBufferSize);

}
CGameFrameWork::~CGameFrameWork() {
	// ���� ���ҽ� ��ȯ
	::closesocket(m_ServerSocket);
	// ���� ����
	::WSACleanup();
}

bool CGameFrameWork::OnCreate(HINSTANCE hInstance, HWND hMainWnd) {
	m_hInstance = hInstance;
	m_hWnd = hMainWnd;

	//Direct3D ����̽� , ��� ť �� ��� ����Ʈ, ����ü�� ���� �����ϴ� �Լ��� ȣ���Ѵ�.
	CreateDirect3DDevice();
	CreateCommandQueueAndList();
	CreateRtvAndDsvDesriptorHeaps();
	CreateSwapChain();
	CreateDepthStencilView();

	BuildObjects();

	return(true);
}

void CGameFrameWork::OnDestroy() {

	WaitForGpuComplete();
	//GPU�� ��� ��� ����Ʈ�� ������ �� ���� ��ٸ���.

	ReleaseObjects();
	//���� ��ü(���� ���� ��ü)�� �Ҹ��Ѵ�.

	::CloseHandle(m_hFenceEvent);

	for (int i = 0; i < m_nSwapChainBuffers; i++) if (m_ppd3dRenderTargetBuffers[i]) m_ppd3dRenderTargetBuffers[i]->Release();
	if (m_pd3dRtvDescriptorHeap) m_pd3dRtvDescriptorHeap->Release();

	if (m_pd3dDepthStencilBuffer) m_pd3dDepthStencilBuffer->Release();
	if (m_pd3dDsvDescriptorHeap) m_pd3dDsvDescriptorHeap->Release();

	if (m_pd3dCommandAllocator) m_pd3dCommandAllocator->Release();
	if (m_pd3dCommandQueue) m_pd3dCommandQueue->Release();
	if (m_pd3dPipelineState) m_pd3dPipelineState->Release();
	if (m_pd3dCommandList) m_pd3dCommandList->Release();

	if (m_pd3dFence) m_pd3dFence->Release();

	m_pdxgiswapChain->SetFullscreenState(FALSE, NULL);
	if (m_pdxgiswapChain) m_pd3dDevice->Release();
	if (m_pd3dDevice)m_pd3dDevice->Release();
	if (m_pdxgiFactory) m_pdxgiFactory->Release();

#if defined(_DEBUG)
	IDXGIDebug1* pdxgiDebug = NULL;
	DXGIGetDebugInterface1(0, __uuidof(IDXGIDebug1), (void**)&pdxgiDebug);
	HRESULT hResult = pdxgiDebug->ReportLiveObjects(DXGI_DEBUG_ALL,
		DXGI_DEBUG_RLO_DETAIL);
	pdxgiDebug->Release();
#endif

}

void CGameFrameWork::CreateSwapChain() {

	RECT rcClient;
	::GetClientRect(m_hWnd, &rcClient);
	m_nWndClientWidth = rcClient.right - rcClient.left;
	m_nWndClientHeight = rcClient.bottom - rcClient.top;

	DXGI_SWAP_CHAIN_DESC1 dxgiSwapChainDesc;
	::ZeroMemory(&dxgiSwapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));
	dxgiSwapChainDesc.Width = m_nWndClientWidth;
	dxgiSwapChainDesc.Height = m_nWndClientHeight;
	dxgiSwapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiSwapChainDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	dxgiSwapChainDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xqualityLevels - 1) : 0;

	dxgiSwapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	dxgiSwapChainDesc.BufferCount = m_nSwapChainBuffers;
	dxgiSwapChainDesc.Scaling = DXGI_SCALING_NONE;
	dxgiSwapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	dxgiSwapChainDesc.AlphaMode = DXGI_ALPHA_MODE_UNSPECIFIED;
	dxgiSwapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;




	DXGI_SWAP_CHAIN_FULLSCREEN_DESC dxgiSwapChainFullScreenDesc;
	::ZeroMemory(&dxgiSwapChainFullScreenDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));
	dxgiSwapChainFullScreenDesc.RefreshRate.Numerator = 60;
	dxgiSwapChainFullScreenDesc.RefreshRate.Denominator = 1;
	dxgiSwapChainFullScreenDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiSwapChainFullScreenDesc.Windowed = TRUE;
	//��üȭ�� ��忡�� ����ȭ���� �ػ󵵸� ����ü��(�ĸ����)�� ũ�⿡ �°� �����Ѵ� +)5�忡�� �߰��� ����

	//HRESULT hResult = m_pdxgiFactory->CreateSwapChain(m_pd3dCommandQueue, &dxgiSwapChainDesc, (IDXGISwapChain**)&m_pdxgiswapChain);
	//m_nSwapChainBufferIndex = m_pdxgiswapChain->GetCurrentBackBufferIndex();


	m_pdxgiFactory->CreateSwapChainForHwnd(m_pd3dCommandQueue, m_hWnd, &dxgiSwapChainDesc, &dxgiSwapChainFullScreenDesc, NULL, (IDXGISwapChain1**)&m_pdxgiswapChain);
	//����ü���� �����Ѵ�.

	m_nSwapChainBufferIndex = m_pdxgiswapChain->GetCurrentBackBufferIndex();
	//����ü���� ���� �ĸ���� �ε����� �����Ѵ�.

	m_pdxgiFactory->MakeWindowAssociation(m_hWnd, DXGI_MWA_NO_ALT_ENTER);
	//ALT+Enter Ű�� ������ ��Ȱ��ȭ�Ѵ�.

#ifndef  _WITH_SWAPCHAIN_FULLCREEND_STATE
	CreateRenderTargetViews();
#endif // ! _WITH_SWAPCHAIN_FULLCREEND_STATE


}

void CGameFrameWork::CreateDirect3DDevice() {
	HRESULT hResult;

	UINT nDXGIFactoryFlags = 0;
#if defined(_DEBUG)
	ID3D12Debug* pd3dDebugController = NULL;
	hResult = D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)&pd3dDebugController);
	if (pd3dDebugController) {

		pd3dDebugController->EnableDebugLayer();
		pd3dDebugController->Release();
	}

	nDXGIFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif



	hResult = ::CreateDXGIFactory2(nDXGIFactoryFlags, __uuidof(IDXGIFactory4), (void**)&m_pdxgiFactory);
	IDXGIAdapter1* pd3dAdapter = NULL;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_pdxgiFactory->EnumAdapters1(i, &pd3dAdapter); i++) {

		DXGI_ADAPTER_DESC1 dxgiAdapterDesc;
		pd3dAdapter->GetDesc1(&dxgiAdapterDesc);
		if (dxgiAdapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_12_0, __uuidof(ID3D12Device), (void**)&m_pd3dDevice))) break;

	}
	//��� �ϵ���� ����� ���Ͽ� Ư�� ���� 12.0�� �����ϴ� �ϵ���� ����̽��� �����Ѵ�.
	if (!pd3dAdapter) {
		m_pdxgiFactory->EnumWarpAdapter(__uuidof(IDXGIAdapter1), (void**)&pd3dAdapter);
		D3D12CreateDevice(pd3dAdapter, D3D_FEATURE_LEVEL_11_0, __uuidof(ID3D12Device), (void**)&m_pd3dDevice);
	}
	//Ư�� ���� 12.0�� �����ϴ� �ϵ���� ����̽��� ������ �� ������ WARP ����̽��� �����Ѵ�.

	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS d3dMsaaQualityLevels;
	d3dMsaaQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dMsaaQualityLevels.SampleCount = 4;
	d3dMsaaQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	d3dMsaaQualityLevels.NumQualityLevels = 0;
	m_pd3dDevice->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &d3dMsaaQualityLevels, sizeof(D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS));
	m_nMsaa4xqualityLevels = d3dMsaaQualityLevels.NumQualityLevels;
	//����̽��� �����ϴ� ���� ������ ǰ�� ������ Ȯ���Ѵ�.
	m_bMsaa4xEnable = (m_nMsaa4xqualityLevels > 1) ? true : false;
	//���� ������ ǰ�� ������ 1���� ũ�� ���� ���ø��� Ȱ��ȭ�Ѵ�.

	hResult = m_pd3dDevice->CreateFence(0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&m_pd3dFence);
	for (UINT i = 0; i < m_nSwapChainBufferIndex; ++i) m_nFenceValues[i] = 0;
	//�潺�� �����ϰ� �潺 ���� 0���� �����Ѵ�.
	m_hFenceEvent = ::CreateEvent(NULL, FALSE, FALSE, NULL);
	//�潺�� ����ȭ�� ���� �̺�Ʈ ��ü�� �����Ѵ�(�̺�Ʈ ��ü�� �ʱⰪ�� false�̴�.) �̺�Ʈ�� ����Ǹ�(Signal) �̺�Ʈ�� ���� �ڵ������� FALSE�� �ǵ��� �����Ѵ�.


	//���� �簢���� �� �������� Ŭ���̾�Ʈ ���� ��ü�� �����Ѵ�.

	if (pd3dAdapter) pd3dAdapter->Release();
}

void CGameFrameWork::CreateCommandQueueAndList() {

	D3D12_COMMAND_QUEUE_DESC d3dCommandQueueDesc;
	::ZeroMemory(&d3dCommandQueueDesc, sizeof(D3D12_COMMAND_QUEUE_DESC));
	d3dCommandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	d3dCommandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	HRESULT hResult = m_pd3dDevice->CreateCommandQueue(&d3dCommandQueueDesc, __uuidof(ID3D12CommandQueue), (void**)&m_pd3dCommandQueue);
	//���� ��� ť�� �����Ѵ�.

	hResult = m_pd3dDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)(&m_pd3dCommandAllocator));
	//���� ��� �Ҵ��ڸ� �����Ѵ�.

	hResult = m_pd3dDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT , m_pd3dCommandAllocator, NULL, __uuidof(ID3D12GraphicsCommandList), (void**)&m_pd3dCommandList);
	//���� ��� ����Ʈ�� �����Ѵ�.

	hResult = m_pd3dCommandList->Close();
	//��� ����Ʈ�� �����Ǹ� ����(open) �����̹Ƿ� ����(Closed) ���·� �����.

}

void CGameFrameWork::CreateRtvAndDsvDesriptorHeaps() {

	D3D12_DESCRIPTOR_HEAP_DESC d3dDescriptorHeapDesc;
	::ZeroMemory(&d3dDescriptorHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));
	d3dDescriptorHeapDesc.NumDescriptors = m_nSwapChainBuffers;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;

	d3dDescriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	d3dDescriptorHeapDesc.NodeMask = 0;
	HRESULT hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dRtvDescriptorHeap);
	//���� Ÿ�� ������ ��(�������� ������ ����ü�� ������ ����)�� �����Ѵ�.
	m_nRtvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	//���� Ÿ�� ������ ���� ������ ũ�⸦ �����Ѵ�.

	d3dDescriptorHeapDesc.NumDescriptors = 1;
	d3dDescriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	hResult = m_pd3dDevice->CreateDescriptorHeap(&d3dDescriptorHeapDesc, __uuidof(ID3D12DescriptorHeap), (void**)&m_pd3dDsvDescriptorHeap);
	//����-���ٽ� ������ ��(�������� ������ 1)�� �����Ѵ�.

	m_nDsvDescriptorIncrementSize = m_pd3dDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	//����-������ ������ ���� ������ ũ�⸦ ���Ѵ�.
}

void CGameFrameWork::CreateRenderTargetViews() {

	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	for (UINT i = 0; i < m_nSwapChainBuffers; ++i) {
		m_pdxgiswapChain->GetBuffer(i, __uuidof(ID3D12Resource), (void**)&m_ppd3dRenderTargetBuffers[i]);
		m_pd3dDevice->CreateRenderTargetView(m_ppd3dRenderTargetBuffers[i], NULL, d3dRtvCPUDescriptorHandle);
		d3dRtvCPUDescriptorHandle.ptr += m_nRtvDescriptorIncrementSize;
	}

}

void CGameFrameWork::CreateDepthStencilView() {
	
	D3D12_RESOURCE_DESC d3dResourceDesc;
	d3dResourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	d3dResourceDesc.Alignment = 0;
	d3dResourceDesc.Width = m_nWndClientWidth;
	d3dResourceDesc.Height = m_nWndClientHeight;
	d3dResourceDesc.DepthOrArraySize = 1; 
	d3dResourceDesc.MipLevels = 1;
	d3dResourceDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dResourceDesc.SampleDesc.Count = (m_bMsaa4xEnable) ? 4 : 1;
	d3dResourceDesc.SampleDesc.Quality = (m_bMsaa4xEnable) ? (m_nMsaa4xqualityLevels - 1) : 0;
	d3dResourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	d3dResourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;
	

	D3D12_HEAP_PROPERTIES d3dHeapProperties;
	::ZeroMemory(&d3dHeapProperties, sizeof(D3D12_HEAP_PROPERTIES));
	d3dHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	d3dHeapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	d3dHeapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	d3dHeapProperties.CreationNodeMask = 1;
	d3dHeapProperties.VisibleNodeMask = 1;
	
	D3D12_CLEAR_VALUE d3dClearValue;
	d3dClearValue.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dClearValue.DepthStencil.Depth = 1.0f;
	d3dClearValue.DepthStencil.Stencil = 0; 
	m_pd3dDevice->CreateCommittedResource(&d3dHeapProperties, D3D12_HEAP_FLAG_NONE, &d3dResourceDesc, D3D12_RESOURCE_STATE_DEPTH_WRITE, &d3dClearValue, __uuidof(ID3D12Resource), (void**)&m_pd3dDepthStencilBuffer);
	//����-���ٽ� ���۸� �����Ѵ�.

	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUdescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	m_pd3dDevice->CreateDepthStencilView(m_pd3dDepthStencilBuffer, NULL, d3dDsvCPUdescriptorHandle);

}




void CGameFrameWork::BuildObjects() {



	m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
	//ī�޶� ��ü�� �����Ͽ� ����Ʈ ,�����簢�� ,���� ��ȯ ��� ,ī�޶� ��ȯ ����� �����ϰ� �����Ѵ�.
	
	//�� ��ü�� �����ϰ� ���� ���Ե� ���� ��ü���� �����Ѵ�.
	//////////////////�Ŵ��� ����//////////////////////////////
	CMeshManager* pMeshManager = new CMeshManager(m_pd3dDevice);

	m_pUIManager = new CUiManager(m_pd3dDevice, m_pd3dCommandList);
	m_pMeshManager = pMeshManager;
	m_pScoreManager = new CUiManager(m_pd3dDevice, m_pd3dCommandList);


	////////////////////���ҽ� �ϰ� GPU�� ���ε�//////////////////////////////////////////////////////////////////
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/REDBUTTON.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/BLUEBUTTON.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/GREENBUTTON.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/YELLOWBUTTON.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/EXITBt.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/StartBt.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/CUSTOMBt.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/StartStage.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Score.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/EnddingMassage.dds");
	//���� ���ε�
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/0.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/1.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/2.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/3.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/4.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/5.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/6.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/7.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/8.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/9.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/alpha.dds");
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//////////////////////////////////////////////////////////////////////////////////////////////////
	//���� ���� UI Update
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/LoginResource/Green.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/LoginResource/NickNameButton.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/LoginResource/LoginButton.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/LoginResource/loginBT.dds");
		m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/LoginResource/nicknameLB.dds");
	//���ĺ� ���ε�
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/MultiPlayButton.dds");
	for (int i = 0; i < 26; ++i) {
		
		std::string addr = "Texture/Alphabet/";
		addr += 'a' + i;
		addr += ".dds";
		m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, addr.c_str());
	}

	//readystage resource
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/readystage/blue.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/readystage/yellow.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/readystage/red.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/readystage/green.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/readystage/ready.dds");
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/readystage/readybutton.dds");
	//playstage resource
	m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/playstage/blue.dds");

	//////////////////////////////////////////////////////////////////////////////////////////////////
	m_pScene = new CScene();
	m_pScene->m_MeshManager = pMeshManager;
	m_pScene->m_pUIManger = m_pUIManager;
	m_pScene->BuildObjects(m_pd3dDevice, m_pd3dCommandList);

	CTanker* pCreater = new CTanker(m_pd3dDevice, m_pd3dCommandList, m_pScene->GetGraphicsRootSignature(), pMeshManager->BringMesh("Models/Missile.bin"),pMeshManager->BringTexture("Texture/ElementBlue.dds"), pMeshManager->BringMesh("Models/Mine.bin"));
	m_pPlayer = pCreater;
	m_pPlayer->m_pMesh = pMeshManager->BringMesh("Models/TankFree.bin");
	m_pPlayer->InitAnimaition();
	pCreater->FindFrameSet();
	pCreater->SetPosition(XMFLOAT3(0.0f, 2.0f, 0.0f));
	m_pPlayer->m_TextureAddr = pMeshManager->BringTexture("Texture/TankBlue.dds");
	pCreater->m_pShader = m_pScene->m_pCllluminatedShader;
	m_pScene->m_pPlayer = pCreater;

	m_pCamera = ((CTanker*)m_pPlayer)->ChangeCamera(THIRD_PERSON_CAMERA, m_GameTimer.GetTimeElapsed());



	//��ư �����
	MakeInitButton();


	//�� ��ü�� �����ϱ� ���Ͽ� �ʿ��� �׷��� ��� ����Ʈ���� ��� ť�� �߰��Ѵ�.
	m_pd3dCommandList->Close();
	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);

	//�׷��� ��� ����Ʈ���� ��� ����� ������ ��ٸ���.

	WaitForGpuComplete();
	//�׷��ȸ��ҽ����� �����ϴ� ������ ������ ������ ���ε� ���۵��� �Ҹ��Ų��.
	if (m_pScene) m_pScene->ReleaseUploadBuffers();

	m_GameTimer.Reset();

}

void CGameFrameWork::ReleaseObjects() {
	if (m_pScene) m_pScene->ReleaseObjects();
	if (m_pScene) delete m_pScene;
}

void CGameFrameWork::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wparam, LPARAM lParam) {

	switch (nMessageID)
	{
	case WM_LBUTTONDOWN:
		m_pSelectedObject = NULL;
		::SetCapture(hWnd);
		::GetCursorPos(&m_ptOldCursorPos);
			break;
	case WM_RBUTTONDOWN:
		//���콺�� �������� ���콺 ��ŷ�� �Ͽ� ������ ���� ��ü�� ã�´�.
		//m_pSelectedObject = m_pScene->PickObjectPointedByCursor(LOWORD(lParam), HIWORD(lParam), m_pCamera);
		//::SetCapture(hWnd);
		//::GetCursorPos(&m_ptOldCursorPos);
		break;
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
		::ReleaseCapture();
		break;
	case WM_MOUSEMOVE:
		break;
	default :
		break;

	}
}

void CGameFrameWork::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
	
	if (wParam ==VK_UP || wParam== VK_DOWN || wParam== VK_LEFT || wParam==VK_RIGHT)
		if (nMessageID == WM_KEYUP)
			((CTanker*)m_pPlayer)->wheelanimation = FALSE;

	if (nMessageID == WM_KEYUP)
	if (/*m_GameState == LoginStage|| */m_GameState==InitStage) {
		if ('A' <= wParam) {
			if ('Z' >= wParam) {
				for (int i = 0; i < NameBufferSize - 1; i++) {
					if (KeyInputBuffer[i] == '\0') {
						KeyInputBuffer[i] = wParam;
						KeyInputBuffer[i + 1] = '\0';
						break;
					}
				}
			}
		}
		else if (VK_BACK == wParam) {
			for (int i = 0; i < NameBufferSize; i++) {
				if (KeyInputBuffer[i] == '\0') {
					if (i != 0) {
						KeyInputBuffer[i - 1] = '\0';
						break;
					}
					else {
						KeyInputBuffer[0] = '\0';

					}
				}
			}
		}
	}


	

	switch (nMessageID) {
	case WM_KEYUP:
		switch (wParam) {
		case VK_ESCAPE:
			::PostQuitMessage(0);
			break;
		case VK_RETURN:
			break;
		case VK_F8:
			break;
		case VK_F9:
			ChangeSwapChainState();
			break;
		case VK_F1:
		case VK_F2:
		case VK_F3:
			if (m_pPlayer) m_pCamera = ((CTanker*)m_pPlayer)->ChangeCamera((wParam - VK_F1 + 1), m_GameTimer.GetTimeElapsed());
			break;
		case VK_CONTROL:
			((CTanker*)m_pPlayer)->FireMissile();
			break;
		case 'X':
			((CTanker*)m_pPlayer)->SetMine();
			break;
		case 'B':
			if (m_bRenderBoundingBox) m_bRenderBoundingBox = false;
			else  m_bRenderBoundingBox = true;
			break;
		case 'R':
			m_pPlayer->SetPosition(XMFLOAT3(0, 2, 0));
		default:
			break;
		}
		break;
	default:
		break;
	}
}

bool CGameFrameWork::OnProcessingUIMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	if (nMessageID == WM_LBUTTONDOWN) {

		long x;
		long y;


		x = LOWORD(lParam);
		y = HIWORD(lParam);

		// if UiManager Exist....
		if (m_pUIManager) {

			// Check Rect Bounding
			for (const auto p : m_pUIManager->FunctionList) {

				//���Ʒ� üũ
				if (p.first->top < y && y < p.first->bottom) {
					if (p.first->left < x && x < p.first->right) {

						switch (p.first->purpose) {
							//�ʱ� �������� ���� ȭ��
						//case LOGIN_BUTTON: //�α��� ��ư     �α��� -> ������������ �̵�
						//	m_GameState = ReadyStage;
						//	m_NickName = KeyInputBuffer;
						//	break;
						case  MULTI_BUTTON:  //�г��� �Է� ȭ������ �̵� 
							//m_GameState = LoginStage;     //�ʱ⽺������ -> �г��� ȭ�� �̵�
							m_GameState = ReadyStage;
							break;
						case START_BUTTON:
							//m_GameState = PlayStage;
							m_GameState = LoginStage;
							break;
						case EXIT_BUTTON:
							// ���� ���ҽ� ��ȯ
							::closesocket(m_ServerSocket);
							// ���� ����
							::WSACleanup();
							exit(0);
							break;
						case CUSTOM_BUTTON:
							m_GameState = CustomStage;
							break;
						case RED_COLOR_BUTTON:
							m_pPlayer->ChangeColor(m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/TankRed.dds"), m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/ElementRed.dds"));
							m_color = red;
							break;
						case BLUE_COLOR_BUTTON:
							m_pPlayer->ChangeColor(m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/TankBlue.dds"), m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/ElementBlue.dds"));
							m_color = blue;
							break;
						case GREEN_COLOR_BUTTON:
							m_pPlayer->ChangeColor(m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/TankGreen.dds"), m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/ElementGreen.dds"));
							m_color = green;
							break;
						case YELLOW_COLOR_BUTTON:
							m_pPlayer->ChangeColor(m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/TankYellow.dds"), m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/ElementYellow.dds"));
							m_color = yellow;
							break;

						case READY_BUTTON:
							if (m_ready) {
								m_ready = false;
							}
							else {
								m_ready = true;
							}
							
							m_GameState = PlayStage;		// �����̰� �˾Ƽ� ����
							break;
						}
					}

				}

			}

		}

		if (m_PreGameState != m_GameState) {
			m_pUIManager->DeleteAllRect();
			m_pScoreManager->DeleteAllRect();
			switch (m_GameState) {
			case PlayStage:
				MakePlayButton();
				break;
			case CustomStage:
				MakeCustomButton();
				break;
			case InitStage:
				break;
			case LoginStage: //�г��� ���ϱ� ��������
				//MakeLoginButton();
				Makemulticustomebutton();
				break;
			case ReadyStage:// ���⿡�� ������ Init�� Connect �ذ�
				MakeReadyStage();

				InitSocket(); //�α���
			}

				m_PreGameState = m_GameState;
		}

	}
	

	return false;
}

LRESULT CALLBACK CGameFrameWork::OnProcessingWindowMessage(HWND hWnd, UINT  nMessageID, WPARAM wParam, LPARAM lParam) {

	switch (nMessageID) {
	case WM_SIZE: {
		m_nWndClientWidth = LOWORD(lParam);
		m_nWndClientHeight = HIWORD(lParam);
		break;
	}
	case WM_LBUTTONDOWN:
	case WM_RBUTTONDOWN:
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MOUSEMOVE:
		//if (m_GameState != PlayStage)
			OnProcessingUIMessage(hWnd, nMessageID, wParam, lParam);
	
			OnProcessingMouseMessage(hWnd, nMessageID, wParam, lParam);
		break;
	case WM_KEYDOWN:
	case WM_KEYUP:
		OnProcessingKeyboardMessage(hWnd, nMessageID, wParam, lParam);
		break;
	}
	return (0);
}

void CGameFrameWork::ProcessInput() {

	static UCHAR pKeyBuffer[256];
	DWORD dwDirection = 0;

	/*Ű������ ���� ������ ��ȯ�Ѵ�. ȭ��ǥ Ű�� ������ �÷��̾ ������ ���� */

	if (::GetKeyboardState(pKeyBuffer)) {


		if (pKeyBuffer[VK_UP] & 0xF0)			dwDirection |= DIR_FORWARD;
		if (pKeyBuffer[VK_DOWN] & 0xF0)			dwDirection |= DIR_BACKWARD;
		if (pKeyBuffer[VK_LEFT] & 0xF0)			m_pPlayer->Rotate(0, -1.0, 0.0f);
		if (pKeyBuffer[VK_RIGHT] & 0xF0)		m_pPlayer->Rotate(0, 1.0, 0.0f);

		if (pKeyBuffer[VK_UP] & 0xF0 || pKeyBuffer[VK_DOWN] & 0xF0 || pKeyBuffer[VK_LEFT] & 0xF0 || pKeyBuffer[VK_RIGHT] & 0xF0) {
			((CTanker*)m_pPlayer)->wheelanimation = TRUE;
			((CTanker*)(m_pPlayer))->flag_move_screen = TRUE;
		}

		//if (pKeyBuffer[VK_LEFT] & 0xF0)			dwDirection |= DIR_LEFT;
		//if (pKeyBuffer[VK_RIGHT] & 0xF0)		dwDirection |= DIR_RIGHT;
		//if (pKeyBuffer[VK_PRIOR] & 0xF0)		dwDirection |= DIR_UP;
		//if (pKeyBuffer[VK_NEXT] & 0xF0)			dwDirection |= DIR_DOWN;
	}

	float cxDelta = 0.0f, cyDelta = 0.0f;
	POINT ptCursorPos;

	if (::GetCapture() == m_hWnd) {

		::SetCursor(NULL);
		((CTanker*)(m_pPlayer))->flag_move_screen = TRUE;
		::GetCursorPos(&ptCursorPos);

		cxDelta = (float)(ptCursorPos.x - m_ptOldCursorPos.x) / 3.0f;
		cyDelta = (float)(ptCursorPos.y - m_ptOldCursorPos.y) / 3.0f;
		//���콺 Ŀ���� ��ġ�� ���콺�� ���ȴ� ��ġ�� �����Ѵ�.

		::SetCursorPos(m_ptOldCursorPos.x, m_ptOldCursorPos.y);
	}

	if ((dwDirection != 0) || (cxDelta != 0) || (cyDelta != 0.0f)) {
		// ��ŷ���� ������ ���� ��ü�� ������ Ű���带 �����ų� ���콺�� �����̸� ���� ��ü�� �̵� �Ǵ� ȸ���Ѵ�. 
		if (m_pSelectedObject)
		{
			ProcessSelectedObject(dwDirection, cxDelta, cyDelta);
		}
		else
		{
			if (cxDelta || cyDelta) {

				if (pKeyBuffer[VK_RBUTTON] & 0xF0)
					m_pPlayer->Rotate(0.0f, 0.0f, 0.0f);
				else 
					m_pPlayer->Rotate(cyDelta, cxDelta, 0.0f);
				

			}

			if (dwDirection) m_pPlayer->Move(dwDirection, 50.0f * m_GameTimer.GetTimeElapsed(), true);
		}
	}
	m_pPlayer->SetBeforePosition(m_pPlayer->GetPosition());
	m_pPlayer->Update(m_GameTimer.GetTimeElapsed());
	//�ӽ������� �浹 üũ�� ����  �ٿ�� �ڽ��� ���� �浹 �˻絵 ����
	if (m_pScene->PlayerToObject()) {
		m_pPlayer->SetPosition(m_pPlayer->GetBeforePosition());
	}


}

void CGameFrameWork::AnimateObjects() {

	if(m_GameState == PlayStage)
		if (m_pScene) m_pScene->AnimateObjects(m_GameTimer.GetTimeElapsed());
	if (m_GameState == CustomStage) {

		XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		//�÷��̾��� ���� y-���� �������� ȸ���ϴ� ����� �����Ѵ�. XMFLOAT3 xmf3Up = m_pPlayer->GetUpVector();
		XMMATRIX xmmtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&xmf3Up), XMConvertToRadians(0.1f));


		//ī�޶��� ��ġ ���͸� �÷��̾� ��ǥ��� ǥ���Ѵ�(������ ����).
		m_pCamera->SetPosition(Vector3::Subtract(m_pCamera->GetPosition(), m_pPlayer->GetPosition()));
		//������ ���� ���͸� ȸ���Ѵ�. 
		m_pCamera->SetPosition(Vector3::TransformCoord(m_pCamera->GetPosition(), xmmtxRotate));
		//ȸ���� ī�޶��� ��ġ�� ���� ��ǥ��� ǥ���Ѵ�.
		m_pCamera->SetPosition(Vector3::Add(m_pCamera->GetPosition(), m_pPlayer->GetPosition()));
		//ī�޶��� ���� x-��, y-��, z-���� ȸ���Ѵ�. 


		//ī�޶��� ���� x-��, y-��, z-���� ȸ�� ����� ����Ͽ� ȸ���Ѵ�. 
		m_pCamera->SetLookVector( Vector3::TransformNormal(m_pCamera->GetLookVector(), xmmtxRotate));
		m_pCamera->SetUpVector( Vector3::TransformNormal(m_pCamera->GetUpVector(), xmmtxRotate));
		m_pCamera->SetRightVector( Vector3::TransformNormal(m_pCamera->GetRightVector(), xmmtxRotate));



		m_pCamera->RegenerateViewMatrix();

	}
}

void CGameFrameWork::WaitForGpuComplete() {


	//cpu �潺�� ���� �����Ѵ�.
	
	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);
	//gpu�� �潺�� ���� �����ϴ� ����� ���ť�� �߰��Ѵ�.
	if (m_pd3dFence->GetCompletedValue() < nFenceValue) {
		//�潺�� ���簪�� ������ ������ ������ �潺�� ���簪�� ������ ���� �ɶ����� ��ٸ���.
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);

	}
}
//#define _WITH_PLAYER_TOP
void CGameFrameWork::FrameAdvance() {






	//Ÿ�̸��� �ð��� ���ŵǵ��� �ϰ� ������ ����Ʈ�� ����Ѵ�.
	m_GameTimer.Tick(0.0f);
	//�ִϸ��̼ǰ� Ű�Է��� �Ұ��ϰ� ����



	HRESULT hResult = m_pd3dCommandAllocator->Reset();
	hResult = m_pd3dCommandList->Reset(m_pd3dCommandAllocator, NULL);
	//��� �Ҵ��ڿ� ��� ����Ʈ�� �����Ѵ�.



	//����Ʈ�� ���� �簢���� �����Ѵ�.
	D3D12_RESOURCE_BARRIER d3dResourceBarrier;
	::ZeroMemory(&d3dResourceBarrier, sizeof(D3D12_RESOURCE_BARRIER));
	d3dResourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	d3dResourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	d3dResourceBarrier.Transition.pResource = m_ppd3dRenderTargetBuffers[m_nSwapChainBufferIndex];
	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);		
	//���� ���� Ÿ�Ͽ� ���� ������Ʈ�� �����⸦ ��ٸ���. ������Ʈ�� ������ ���� Ÿ�� ������ ���´� ������Ʈ ���¿��� ���� Ÿ�� ���·� �ٲ� ���̴�.


	D3D12_CPU_DESCRIPTOR_HANDLE d3dRtvCPUDescriptorHandle = m_pd3dRtvDescriptorHeap -> GetCPUDescriptorHandleForHeapStart();
	d3dRtvCPUDescriptorHandle.ptr += (m_nSwapChainBufferIndex * m_nRtvDescriptorIncrementSize);

	//������ ���� Ÿ�Ͽ� �ش��ϴ� �������� cpu �������� cpu�ּҸ� ����Ѵ�.

	float pfClearColor[4] = { 0.0f, 0.125f , 0.3f , 1.0f };

	m_pd3dCommandList->ClearRenderTargetView(d3dRtvCPUDescriptorHandle, pfClearColor, 0, NULL);


	D3D12_CPU_DESCRIPTOR_HANDLE d3dDsvCPUDescriptorHandle = m_pd3dDsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	//���� - ���ٽ� �������� cpu �ּҸ� ����Ѵ�.



	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	//���ϴ� ������ ����- ���ٽ�(��)�� �����.

	m_pd3dCommandList->OMSetRenderTargets(1, &d3dRtvCPUDescriptorHandle, TRUE, &d3dDsvCPUDescriptorHandle);

	//������ �ڵ�� ���⿡ �߰� �� ����.
	//6�忡�� �߰�..



	//3��Ī ī�޶��϶� �÷��̾ �׻� ���̵��� ������ �Ѵ�.
#ifdef _WITH_PLAYER_TOP
	//���� Ÿ���� �״�� �ΰ� ���� ���۸� 1.0���� ����� �÷��̾ ������ �ϸ� �÷��̾�� ������ �׷������̴�.
	m_pd3dCommandList->ClearDepthStencilView(d3dDsvCPUDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
#endif


	
	if (m_GameState == PlayStage)
		{
			AnimateObjects();
			ProcessInput();
			ChangeScore();
	}
	else if (m_GameState == CustomStage) {
		AnimateObjects();
	}
	else if (m_GameState == InitStage) {
		ShowInputName();

	}


	if (m_pScene) m_pScene->Render(m_pd3dCommandList, m_pCamera);

	if (m_pPlayer) ((CTanker*)(m_pPlayer))->Render(m_pd3dCommandList, m_pCamera);


	if (m_bRenderBoundingBox) m_pScene->RenderBoundingBox(m_pd3dCommandList, m_pCamera);


	if (m_pUIManager) m_pUIManager->AlDrawRect(m_pd3dCommandList);
	if (m_pScoreManager) m_pScoreManager->AlDrawRect(m_pd3dCommandList);
	
	//�¸� ���� �߰�
	if (m_pPlayer->m_Score == m_pScene->CTankObjects.size()) {
		m_pUIManager->DeleteAllRect();
		m_pScoreManager->DeleteAllRect();
		m_GameState = EndStage;
		m_PreGameState = EndStage;
		
		MakeEndButton();
	}

	d3dResourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	d3dResourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	d3dResourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	m_pd3dCommandList->ResourceBarrier(1, &d3dResourceBarrier);

	//���� ���� Ÿ�ٿ� ���� ���������� �����⸦ ��ٸ���. gpu�� ����Ÿ���� ���̻� ������� ������ ���� Ÿ���� ���´� ������Ʈ���·� �ٲ���̴�.

	hResult = m_pd3dCommandList->Close();
	//��� ����Ʈ�� ��� ť�� �߰��Ͽ� �����Ѵ�.

	ID3D12CommandList* ppd3dCommandLists[] = { m_pd3dCommandList };
	m_pd3dCommandQueue->ExecuteCommandLists(1, ppd3dCommandLists);
	//��� ����Ʈ�� ��� ť�� �߰��Ͽ� �����Ѵ�.

	WaitForGpuComplete();
	//GPU�� ��� ��ɸ���Ʈ�� ������ �� ���� ��ٸ���.


	m_pdxgiswapChain->Present(0,0);
	MoveToNextFrame();

	//����ü���� ������Ʈ�Ѵ�. ������Ʈ�� �ϸ� ���� ����Ÿ��(�ĸ����)�� ������ ������۷� �Ű����� ���� Ÿ�� �ε����� �ٱа��̴�.
	::_itow_s(m_GameTimer.GetFrameRate(), (m_pszFrameRate + 12), 37, 10);
	::wcscat_s((m_pszFrameRate + 12), 37, _T(" FPS)"));
	m_nSwapChainBufferIndex = m_pdxgiswapChain->GetCurrentBackBufferIndex();
	m_GameTimer.GetFrameRate(m_pszFrameRate + 12, 37);
	::SetWindowText(m_hWnd, m_pszFrameRate);
}

void CGameFrameWork::ChangeSwapChainState() {
	WaitForGpuComplete();

	BOOL bFullSscreenState = FALSE;

	m_pdxgiswapChain->GetFullscreenState(&bFullSscreenState, NULL);
	m_pdxgiswapChain->SetFullscreenState(!bFullSscreenState, NULL);

	DXGI_MODE_DESC dxgiTargetParameters;
	dxgiTargetParameters.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiTargetParameters.Width = m_nWndClientWidth;
	dxgiTargetParameters.Height = m_nWndClientHeight;
	dxgiTargetParameters.RefreshRate.Numerator = 60;
	dxgiTargetParameters.RefreshRate.Denominator = 1;
	dxgiTargetParameters.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	dxgiTargetParameters.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	m_pdxgiswapChain->ResizeTarget(&dxgiTargetParameters);

	for (int i = 0; i < m_nSwapChainBuffers; ++i) if (m_ppd3dRenderTargetBuffers[i]) m_ppd3dRenderTargetBuffers[i]->Release();

	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDesc;
	m_pdxgiswapChain->GetDesc(&dxgiSwapChainDesc);
	m_pdxgiswapChain->ResizeBuffers(m_nSwapChainBuffers, m_nWndClientWidth,m_nWndClientHeight,dxgiSwapChainDesc.BufferDesc.Format ,dxgiSwapChainDesc.Flags);

	m_nSwapChainBufferIndex = m_pdxgiswapChain->GetCurrentBackBufferIndex();

	CreateRenderTargetViews();



}

void CGameFrameWork::MoveToNextFrame() {

	m_nSwapChainBufferIndex = m_pdxgiswapChain->GetCurrentBackBufferIndex();

	UINT64 nFenceValue = ++m_nFenceValues[m_nSwapChainBufferIndex];
	HRESULT hResult = m_pd3dCommandQueue->Signal(m_pd3dFence, nFenceValue);

	if (m_pd3dFence->GetCompletedValue() < nFenceValue) {
		
		hResult = m_pd3dFence->SetEventOnCompletion(nFenceValue, m_hFenceEvent);
		::WaitForSingleObject(m_hFenceEvent, INFINITE);
	}
}

void CGameFrameWork::ProcessSelectedObject(DWORD dwDirection, float cxDelta, float
	cyDelta)
{
	//��ŷ���� ������ ���� ��ü�� ������ Ű���带 �����ų� ���콺�� �����̸� ���� ��ü�� �̵� �Ǵ� ȸ���Ѵ�. 
	if (dwDirection != 0)
	{
		if (dwDirection & DIR_FORWARD) m_pSelectedObject->MoveForward(+1.0f);
		if (dwDirection & DIR_BACKWARD) m_pSelectedObject->MoveForward(-1.0f);
		if (dwDirection & DIR_LEFT) m_pSelectedObject->MoveStrafe(+1.0f);
		if (dwDirection & DIR_RIGHT) m_pSelectedObject->MoveStrafe(-1.0f);
		if (dwDirection & DIR_UP) m_pSelectedObject->MoveUp(+1.0f);
		if (dwDirection & DIR_DOWN) m_pSelectedObject->MoveUp(-1.0f);
	}
	else if ((cxDelta != 0.0f) || (cyDelta != 0.0f))
	{
		m_pSelectedObject->Rotate(cyDelta, cxDelta, 0.0f);
	}

}

void CGameFrameWork::Makemulticustomebutton() {
	m_pUIManager->CreateUIRect_Func(400, 500, 600, 800, CUSTOM_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/CUSTOMBt.dds"), NULL);
	m_pUIManager->CreateUIRect_Func(400, 600, 350, 550, MULTI_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/MultiPlayButton.dds"), NULL);
	m_pUIManager->CreateUIRect(0, FRAME_BUFFER_HEIGHT, 0, FRAME_BUFFER_WIDTH, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/StartStage.dds"));
}


void CGameFrameWork::MakeInitButton()
{
	if (m_pMeshManager && m_pUIManager) {
		
		
		MakeLoginButton();
		ShowInputName();

		m_pUIManager->CreateUIRect(435, 535, 50, 200, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/LoginResource/nicknameLB.dds"));
		m_pUIManager->CreateUIRect(475, 510, 210, 790, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/LoginResource/Green.dds"));
	

		m_pUIManager->CreateUIRect_Func(440, 540, 800, 1050, START_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/StartBt.dds"), NULL);
		//m_pUIManager->CreateUIRect_Func(600, 700, 350, 530, CUSTOM_BUTTON,			m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/CUSTOMBt.dds"), NULL);
		m_pUIManager->CreateUIRect_Func(550, 650, 830, 1030, EXIT_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/EXITBt.dds"), NULL);
		//m_pUIManager->CreateUIRect_Func(490, 700, 150, 350, MULTI_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/MultiPlayButton.dds"), NULL);



		m_pUIManager->CreateUIRect(0, FRAME_BUFFER_HEIGHT, 0, FRAME_BUFFER_WIDTH, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/StartStage.dds"));
		
	}

}

void CGameFrameWork::MakePlayButton()
{
	if (m_pMeshManager && m_pUIManager) {
		//m_pUIManager->CreateUIRect_Func(0, 50, 0, 100, CUSTOM_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/CUSTOMBt.dds"), NULL);
		m_pUIManager->CreateUIRect_Func(0, 50, FRAME_BUFFER_WIDTH - 100, FRAME_BUFFER_WIDTH, EXIT_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/EXITBt.dds"), NULL);
		m_pUIManager->CreateUIRect(0, 50, 270, 400, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Score.dds"));
		//3�ڸ�����.
		m_pScoreManager->CreateUIRect(5, 45, 410, 440, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/alpha.dds"));
		m_pScoreManager->CreateUIRect(5, 45, 450, 480, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/alpha.dds"));
		m_pScoreManager->CreateUIRect(5, 45, 490, 520, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/alpha.dds"));

	}

	//player info

	//nickname
	char filename[50];
	for (int i = 0; i < m_NickName.length(); i++) {
		std::string temp = "Texture/Alphabet/";
		temp += m_NickName[i];
		for (int i = 0; i < temp.length(); i++) {
			filename[i] = temp[i];
			filename[temp.length()] = '\0';
		}
		// ���ĺ� ���� -> ���� ����:30,���� ����:25 (����:���ĺ� �Ѱ�)
		m_pUIManager->CreateUIRect(600, 630, 120 + 25 * i, 145 + 25 * i, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, filename));
	}

	//color
	m_pUIManager->CreateUIRect(540, 650, 0, 180, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/playstage/blue.dds"));



}

void CGameFrameWork::MakeCustomButton()
{

	if (m_pMeshManager && m_pUIManager) {


		m_pUIManager->CreateUIRect_Func(600, 650, 350, 440, RED_COLOR_BUTTON,   m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/REDBUTTON.dds")    , NULL);
		m_pUIManager->CreateUIRect_Func(600, 650, 440, 530, GREEN_COLOR_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/GREENBUTTON.dds") , NULL);
		m_pUIManager->CreateUIRect_Func(650, 700, 350, 440, BLUE_COLOR_BUTTON,  m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/BLUEBUTTON.dds")  , NULL);
		m_pUIManager->CreateUIRect_Func(650, 700, 440, 530, YELLOW_COLOR_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/YELLOWBUTTON.dds"), NULL);
		m_pUIManager->CreateUIRect_Func(600, 700, 350, 530, CUSTOM_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/CUSTOMBt.dds"), NULL);
		m_pUIManager->CreateUIRect_Func(490, 590, 350, 700, START_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/StartBt.dds"), NULL);
		m_pUIManager->CreateUIRect_Func(600, 700, 540, 700, EXIT_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/EXITBt.dds"), NULL);
	}

}

void CGameFrameWork::MakeEndButton()
{	

	m_pUIManager->CreateUIRect(0, FRAME_BUFFER_HEIGHT,0, FRAME_BUFFER_WIDTH, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/EnddingMassage.dds"));

	m_pUIManager->CreateUIRect_Func(600, 700, FRAME_BUFFER_WIDTH/2-100, FRAME_BUFFER_WIDTH/2+100, EXIT_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/EXITBt.dds"), NULL);

}
//�α��� ��
void CGameFrameWork::MakeLoginButton() {
	//15���ڱ���
	for (int i = 0; i < NameBufferSize - 1; i++)
	{
		m_pUIManager->CreateUIRect(480, 510, 220 + 25 * i, 250 + 25 * i, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/Alphabet/a.dds"));
		//m_pUIManager->CreateUIRect(485, 515, 310 , 330, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/alpha.dds"));
	}
	//���ĺ�
	//m_pUIManager->CreateUIRect(450, 750, 320, 370, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/Alphabet/a.dds"));
	//
	//m_pUIManager->CreateUIRect_Func(480, 520, 810, 1010, LOGIN_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/LoginResource/loginBT.dds"), NULL);
	//m_pUIManager->CreateUIRect(420, 570, 100, 300, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/LoginResource/nicknameLB.dds"));
	//m_pUIManager->CreateUIRect(480, 520, 310, 800, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/LoginResource/Green.dds"));
	//m_pUIManager->CreateUIRect(0, FRAME_BUFFER_HEIGHT, 0, FRAME_BUFFER_WIDTH, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/StartStage.dds"));

}
void CGameFrameWork::MakeReadyStage() {
	
	
	//�г���
	char filename[50];
		for (int i = 0; i < m_NickName.length(); i++) {
		std::string temp = "Texture/Alphabet/";
		temp += m_NickName[i];
		for (int i = 0; i < temp.length(); i++) {
			filename[i] = temp[i];
			filename[temp.length()] = '\0';
		}
		// ���ĺ� ���� -> ���� ����:30,���� ����:25
		m_pUIManager->CreateUIRect(90, 120, 460 + 25 * i, 485 + 25 * i, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, filename));
	}

	//ready
	if (m_ready) {

	}
	else {
		m_pUIManager->CreateUIRect(20, 70, 515, 565, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/ready.dds"));
	}

	//color 
	m_pUIManager->CreateUIRect(0, 300, 270, 810, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/readystage/blue.dds"));

	m_pUIManager->CreateUIRect_Func(620, 680, 690, 870, READY_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/readystage/readybutton.dds"), NULL);
	m_pUIManager->CreateUIRect_Func(620, 680, 880,1060 , EXIT_BUTTON, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/EXITBt.dds"), NULL);
	//background
	m_pUIManager->CreateUIRect(0, FRAME_BUFFER_HEIGHT, 0, FRAME_BUFFER_WIDTH, m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/StartStage.dds"));
	


}




void CGameFrameWork::ShowInputName()
{
	auto pUI = m_pUIManager->RectList.begin();
	for (int i = 0; i < NameBufferSize - 1; i++) {
		if (KeyInputBuffer[i] != '\0') {
			std::string temp ="Texture/Alphabet/";
			temp+=KeyInputBuffer[i];
			temp += ".dds";
			pUI->second = m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, temp.c_str());
		}
		else {
			pUI->second = m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/alpha.dds");
		}
		pUI++;
	}
	pUI = m_pUIManager->RectList.begin();


	for (int i = 0; KeyInputBuffer[i] != '\0'; i++) {
		m_NickName[i] = KeyInputBuffer[i];
	}


}


//void CGameFrameWork:
void CGameFrameWork::ChangeScore()
{	
	int Score = m_pPlayer->m_Score;
	std::string SScore = to_string(Score);
	auto p = m_pScoreManager->RectList.begin();
	if (m_pScoreManager->RectList.size()>2) {
		
		for (int i = 0; i < SScore.size(); i++) {
			 p->second = GeneratorNumTex((SScore[i] - '0'));
			 p++;
		}
		for (int i = SScore.size(); i < 3; i++) {
			p->second = GeneratorNumTex(-1);
			if (i != 2)
				p++;
		}	
		
	}
}


CD3DX12_GPU_DESCRIPTOR_HANDLE* CGameFrameWork::GeneratorNumTex(int num)
{
	int DigitCount = 0;
	switch (num) {
	case 0:
		return m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/0.dds");
	case 1:
		return m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/1.dds");
	case 2:
		return m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/2.dds");
	case 3:
		return m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/3.dds");
	case 4:
		return m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/4.dds");
	case 5:
		return m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/5.dds");
	case 6:
		return m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/6.dds");
	case 7:
		return m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/7.dds");
	case 8:
		return m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/8.dds");
	case 9:
		return m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/9.dds");
	default:
		return m_pMeshManager->BringTexture(m_pd3dDevice, m_pd3dCommandList, "Texture/UIResource/Number/alpha.dds");

	}

}

//�α��� �õ� �� ������ ���� �õ�.
int CGameFrameWork::InitSocket() {
	int retval;

	// ���� �ʱ�ȭ
	WSADATA wsa;
	if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
		return 1;

	// ���� ����
	m_ServerSocket = ::socket(AF_INET, SOCK_STREAM, 0);


	// connect()
	struct sockaddr_in serveraddr;
	memset(&serveraddr, 0, sizeof(serveraddr));
	serveraddr.sin_family = AF_INET;
	inet_pton(AF_INET, SERVERIP, &serveraddr.sin_addr);
	serveraddr.sin_port = htons(PORT_NUM);

	retval = connect(m_ServerSocket, (struct sockaddr*)&serveraddr, sizeof(serveraddr));



	//�Է��� �г��� ����.
	CS_LOGIN_PACKET p;
	p.size = sizeof(CS_LOGIN_PACKET);
	p.type = CS_LOGIN;
	strcpy(p.name, m_NickName.c_str());
	p.name[m_NickName.length()] = '\0';
	memcpy(m_SendBuffer, reinterpret_cast<char*>(&p), sizeof(CS_LOGIN_PACKET));

	send(m_ServerSocket, m_SendBuffer, sizeof(m_SendBuffer), 0); //�г��� ����
	
	int recvLen = ::recv(m_ServerSocket, m_RecvBuffer, sizeof(m_RecvBuffer), 0);
	if (recvLen <= 0)
	{
		int errCode = ::WSAGetLastError();
		cout << "Bind ErrorCode : " << errCode << endl;
	}
	process_packet(0, m_RecvBuffer); //���ú� ������ ���� ���μ��� ��Ŷ���� ó������

	//this_thread::sleep_for(100s);



	return 0;

}