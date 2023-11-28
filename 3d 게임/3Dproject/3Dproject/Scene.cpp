#include "stdafx.h"
#include "Scene.h"
#include "Shader.h"
#include "GameObject.h"


//1000000 x 1000000 x 1000000 싸이즈의 스카이맵


void CScene::BuildDefaultLightsAndMaterials()
{
	m_nLights = 4;
	m_pLights = new LIGHT[m_nLights];
	::ZeroMemory(m_pLights, sizeof(LIGHT) * m_nLights);

	m_xmf4GlobalAmbient = XMFLOAT4(0.15f, 0.15f, 0.15f, 1.0f);

	m_pLights[0].m_bEnable = true;
	m_pLights[0].m_nType = POINT_LIGHT;
	m_pLights[0].m_fRange = 1000.0f;
	m_pLights[0].m_xmf4Ambient = XMFLOAT4(0.1f, 0.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.0f, 0.0f, 1.0f);
	m_pLights[0].m_xmf4Specular = XMFLOAT4(0.5f, 0.5f, 0.5f, 0.0f);
	m_pLights[0].m_xmf3Position = XMFLOAT3(30.0f, 30.0f, 30.0f);
	m_pLights[0].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 0.0f);
	m_pLights[0].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.001f, 0.0001f);
	m_pLights[1].m_bEnable = true;
	m_pLights[1].m_nType = SPOT_LIGHT;
	m_pLights[1].m_fRange = 500.0f;
	m_pLights[1].m_xmf4Ambient = XMFLOAT4(0.1f, 0.1f, 0.1f, 1.0f);
	m_pLights[1].m_xmf4Diffuse = XMFLOAT4(0.4f, 0.4f, 0.4f, 1.0f);
	m_pLights[1].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[1].m_xmf3Position = XMFLOAT3(-50.0f, 20.0f, -5.0f);
	m_pLights[1].m_xmf3Direction = XMFLOAT3(0.0f, 0.0f, 1.0f);
	m_pLights[1].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[1].m_fFalloff = 8.0f;
	m_pLights[1].m_fPhi = (float)cos(XMConvertToRadians(40.0f));
	m_pLights[1].m_fTheta = (float)cos(XMConvertToRadians(20.0f));
	m_pLights[2].m_bEnable = true;
	m_pLights[2].m_nType = DIRECTIONAL_LIGHT;
	m_pLights[2].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[2].m_xmf4Diffuse = XMFLOAT4(0.8f, 0.8f, 0.8f, 1.0f);
	m_pLights[2].m_xmf4Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 0.0f);
	m_pLights[2].m_xmf3Direction = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_pLights[3].m_bEnable = true;
	m_pLights[3].m_nType = SPOT_LIGHT;
	m_pLights[3].m_fRange = 600.0f;
	m_pLights[3].m_xmf4Ambient = XMFLOAT4(0.3f, 0.3f, 0.3f, 1.0f);
	m_pLights[3].m_xmf4Diffuse = XMFLOAT4(0.3f, 0.7f, 0.0f, 1.0f);
	m_pLights[3].m_xmf4Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
	m_pLights[3].m_xmf3Position = XMFLOAT3(50.0f, 30.0f, 30.0f);
	m_pLights[3].m_xmf3Direction = XMFLOAT3(0.0f, 1.0f, 1.0f);
	m_pLights[3].m_xmf3Attenuation = XMFLOAT3(1.0f, 0.01f, 0.0001f);
	m_pLights[3].m_fFalloff = 8.0f;
	m_pLights[3].m_fPhi = (float)cos(XMConvertToRadians(90.0f));
	m_pLights[3].m_fTheta = (float)cos(XMConvertToRadians(30.0f));
}

void CScene::CreateGraphicsPipelineState(ID3D12Device* pd3dDevice) {
	//정점 쉐이더와 픽셀 쉐이더를 생성한다.
	ID3DBlob* pd3dVertexShaderBlob = NULL;
	ID3DBlob* pd3dPixelShaderBlob = NULL;

	UINT nCompileFlags = 0;
#if defined(_DEBUG)
	nCompileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#endif

	D3DCompileFromFile(L"Shaders.hlsl", NULL, NULL, "VSMain", "vs_5_1", nCompileFlags, 0, &pd3dVertexShaderBlob, NULL);
	D3DCompileFromFile(L"shaders.hlsl", NULL, NULL, "PSMain", "ps_5_1", nCompileFlags, 0, &pd3dPixelShaderBlob, NULL);

	//레스터라이저 상태를 설정한다.
	D3D12_RASTERIZER_DESC d3dRasterizerDesc;
	::ZeroMemory(&d3dRasterizerDesc, sizeof(D3D12_RASTERIZER_DESC));
	d3dRasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	d3dRasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	d3dRasterizerDesc.FrontCounterClockwise = FALSE;
	d3dRasterizerDesc.DepthBias = 0;
	d3dRasterizerDesc.DepthBiasClamp = 0.0f;
	d3dRasterizerDesc.SlopeScaledDepthBias = 0.0f;
	d3dRasterizerDesc.DepthClipEnable = TRUE;
	d3dRasterizerDesc.MultisampleEnable = FALSE;
	d3dRasterizerDesc.AntialiasedLineEnable = FALSE;
	d3dRasterizerDesc.ForcedSampleCount = 0;
	d3dRasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	//블랜드 상태를 설정한다.
	D3D12_BLEND_DESC d3dBlendDesc;
	::ZeroMemory(&d3dBlendDesc, sizeof(D3D12_BLEND_DESC));
	d3dBlendDesc.AlphaToCoverageEnable = FALSE;
	d3dBlendDesc.IndependentBlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].BlendEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	d3dBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	d3dBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	d3dBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	d3dBlendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	d3dBlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	//그래픽 팔이프라인 상태를 설정한다.
	D3D12_GRAPHICS_PIPELINE_STATE_DESC d3dPipelineStateDesc;
	::ZeroMemory(&d3dPipelineStateDesc, sizeof(D3D12_GRAPHICS_PIPELINE_STATE_DESC));
	d3dPipelineStateDesc.pRootSignature = m_pd3dGraphicsRootSignature;
	d3dPipelineStateDesc.VS.pShaderBytecode = pd3dVertexShaderBlob->GetBufferPointer();
	d3dPipelineStateDesc.VS.BytecodeLength = pd3dVertexShaderBlob->GetBufferSize();

	d3dPipelineStateDesc.PS.pShaderBytecode = pd3dPixelShaderBlob->GetBufferPointer();
	d3dPipelineStateDesc.PS.BytecodeLength = pd3dPixelShaderBlob->GetBufferSize();
	d3dPipelineStateDesc.RasterizerState = d3dRasterizerDesc;
	d3dPipelineStateDesc.BlendState = d3dBlendDesc;
	d3dPipelineStateDesc.DepthStencilState.DepthEnable = FALSE;
	d3dPipelineStateDesc.DepthStencilState.StencilEnable = FALSE;
	d3dPipelineStateDesc.InputLayout.pInputElementDescs = NULL;
	d3dPipelineStateDesc.InputLayout.NumElements = 0;
	d3dPipelineStateDesc.SampleMask = UINT_MAX;
	d3dPipelineStateDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	d3dPipelineStateDesc.NumRenderTargets = 1;
	d3dPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	d3dPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	d3dPipelineStateDesc.SampleDesc.Count = 1;
	d3dPipelineStateDesc.SampleDesc.Quality = 0;
	pd3dDevice->CreateGraphicsPipelineState(&d3dPipelineStateDesc, __uuidof(ID3D12PipelineState), (void**)&m_pd3dPipelineState);


	if (pd3dVertexShaderBlob) pd3dVertexShaderBlob->Release();
	if (pd3dPixelShaderBlob) pd3dPixelShaderBlob->Release();

}

void CScene::LoadSceneObjectsFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, char* pstrFileName)
{
	FILE* pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);



	char pstrToken[256] = { '\0' };
	char pstrGameObjectName[256] = { '\0' };

	UINT nReads = 0;
	BYTE nStrLength = 0, nObjectNameLength = 0;

	int m_nObjects = 0;

	::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //"<GameObjects>:"
	nReads = (UINT)::fread(&m_nObjects, sizeof(int), 1, pFile);

	//m_ppObjects = new CGameObject * [m_nObjects];


	CGameObject* pGameObject = NULL;
	for (int i = 0; i < m_nObjects; i++)
	{
		pGameObject = new CGameObject();

		::ReadUnityBinaryString(pFile, pstrToken, &nStrLength); //"<GameObject>:"
		::ReadUnityBinaryString(pFile, pstrGameObjectName, &nObjectNameLength);
		pstrGameObjectName[nObjectNameLength] = '\0';
		strcpy_s(pGameObject->m_pstrName, 256, pstrGameObjectName);

		nReads = (UINT)::fread(&pGameObject->m_xmf4x4World, sizeof(float), 16, pFile); //Transform
		/*	XMFLOAT4X4 temp4x4 = Matrix4x4::Identity();
			temp4x4._41 = pGameObject->m_xmf4x4World._41;
			temp4x4._42 = pGameObject->m_xmf4x4World._42;
			temp4x4._43 = pGameObject->m_xmf4x4World._43;
			pGameObject->m_xmf4x4World = temp4x4;*/
		CSumMesh* pMesh = NULL;

		string pstrFilePath = string("Models/Meshes/") + string(pstrGameObjectName) + string(".bin");
		string TextureFilePath = string("Texture/") + string(pstrGameObjectName) + string(".dds");
		//카메라에 넣어서 그런것같음
		if (string("Models/Meshes/Cube.bin") != string(pstrFilePath)) {
			pMesh = m_MeshManager->BringMesh(pd3dDevice, pd3dCommandList, pd3dRootSignature, pstrFilePath.c_str());


			pGameObject->SetMesh(pMesh);
			pGameObject->m_TextureAddr = m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, TextureFilePath.c_str());
			pGameObject->UpdateBoundingBox();
			CGameObjects.push_back(pGameObject);
		}
		else {
			pMesh = m_MeshManager->BringMesh(pd3dDevice, pd3dCommandList, pd3dRootSignature, pstrFilePath.c_str());


			pGameObject->SetMesh(pMesh);
			pGameObject->m_TextureAddr = m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/Sandstone.dds");
			pGameObject->UpdateBoundingBox();
			m_CGameBackGround.push_back(pGameObject);



		}



	}

	::fclose(pFile);
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {


	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	m_pCllluminatedShader = (CIlluminatedShader*)CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);


	m_pBoundingBoxShader = new CBoundingBoxShader();
	m_pBoundingBoxShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_LINE);
	m_pUIManger->m_pShader->CreateShader(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);




	CreateShaderVariables(pd3dDevice, pd3dCommandList);

	//리소스 업로드
	m_MeshManager->BringMesh(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Models/Missile.bin");
	m_MeshManager->BringMesh(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Models/Mine.bin");
	//m_MeshManager->BringMesh(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Desert_Building.bin");
	//m_MeshManager->BringMesh(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Model/Desert_Building_Destroyed.bin");


	m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/box.dds");
	m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/TankBlue.dds");
	m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/TankRed.dds");
	m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/TankGreen.dds");
	m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/TankYellow.dds");
	m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/ElementBlue.dds");
	m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/ElementGreen.dds");
	m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/ElementRed.dds");
	m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/ElementYellow.dds");
	////////////////////
	// 스카이 구 
	///////////////////
	CGameObject* pGameObject = new CGameObject;
	pGameObject->SetMesh(m_MeshManager->BringMesh(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Models/Sphere.bin"));
	pGameObject->m_TextureAddr = m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/Skybox.dds");
	pGameObject->SetPosition(XMFLOAT3(0, -100000, 0));
	pGameObject->UpdateBoundingBox();
	CGameObjects.push_back(pGameObject);


	////////////////////
	//씬 노드하기
	///////////////////
	LoadSceneObjectsFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Scene.bin");


	////////////////////
   //적 오브젝트
   ///////////////////
	for (int i = 0; i < 4; i++) {
		CTank* pGameObject = new CTank(m_MeshManager->BringMesh(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, "Models/TankFree.bin"));
		pGameObject->m_TextureAddr = m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/TankRed.dds");
		bool CheckCrush = true;
		pGameObject->m_bActive = false;
		int test = 0;
		if (test == 0) {
			while (CheckCrush) {
				pGameObject->SetPosition(XMFLOAT3(RANDOM_SIGN * rand() % 1000, 2, RANDOM_SIGN * rand() % 1000));
				pGameObject->UpdateBoundingBox();
				for (CGameObject* pObejct : m_CGameBackGround) {
					if (!pObejct->m_BoundingBox.Intersects(pGameObject->m_BoundingBox)) {
						CheckCrush = false;
					}
				}
			}
		}
		else {
			pGameObject->SetPosition(rand() % 100, 2, rand() % 100);
			pGameObject->UpdateBoundingBox();
		}

		//행렬 정보 얻기
		pGameObject->InitAnimaition();
		pGameObject->FindFrameSet();
		CTankObjects.push_back(pGameObject);


	}

	//Explode입니당
	for (int i = 0; i < 4; i++) {
		BillBoard* pBillboard = new BillBoard();
		pBillboard->m_bActive = FALSE;
		pBillboard->SetMesh(new CSumMesh());
		pBillboard->SetPosition(XMFLOAT3(0, 3, 0));
		pBillboard->SettedTimer = 0.02f;
		pBillboard->m_BillMesh = new CBillboardMesh(pd3dDevice, pd3dCommandList);
		pBillboard->m_BillMesh->UpdataVertexPosition(UIRect(-3, 3, -3, 3), 0);
		pBillboard->SetRowNCol(8, 8);
		pBillboard->m_TextureAddr = m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/Effect/Explode_8x8.dds");
		m_BillBoardList.push_back(pBillboard);
	}

	//연기 구현 
	for (int i = 0; i < 24; i++) {
		BillBoard* pBillboard = new BillBoard();
		pBillboard->m_bActive = FALSE;
		pBillboard->SetMesh(new CSumMesh());
		pBillboard->SetPosition(XMFLOAT3(0, 3, 0));
		pBillboard->SettedTimer = 0.08f;
		pBillboard->m_BillMesh = new CBillboardMesh(pd3dDevice, pd3dCommandList);
		pBillboard->m_BillMesh->UpdataVertexPosition(UIRect(-3, 3, -3, 3), 0);
		pBillboard->SetRowNCol(6, 6);
		pBillboard->m_TextureAddr = m_MeshManager->BringTexture(pd3dDevice, pd3dCommandList, "Texture/Effect/Explosion.dds");
		m_SubBillBoardList.push_back(pBillboard);
	}

	//모든 총알 초기화

	for (int i = 0; i < MAX_USER; ++i) {

		for (int j = 0; j < BULLETS; ++j) {
			AllBullets[i][j].m_pMesh = m_MeshManager->BringMesh("Models/Missile.bin");
			AllBullets[i][i].m_TextureAddr = m_MeshManager->BringTexture("Texture/ElementBlue.dds");
			AllBullets[i][j].m_bActive = false;
		}
	}

	for (int i = 0; i < MAX_USER; ++i) {

		for (int j = 0; j < MINES; ++j) {
			AllMines[i][j].SetMesh(m_MeshManager->BringMesh("Models/Mine.bin"));
			AllMines[i][i].m_TextureAddr = m_MeshManager->BringTexture("Texture/ElementBlue.dds");
			AllMines[i][j].m_bActive = false;
			AllMines[i][j].m_pMesh->m_pMesh->m_xmBoundingBox.Extents.y += 1.0f;
		}
	}
}

void CScene::ReleaseObjects() {
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();


}
//7장 추가
void CScene::ReleaseUploadBuffers() {


}

ID3D12RootSignature* CScene::GetGraphicsRootSignature() {
	return m_pd3dGraphicsRootSignature;
}


void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	::memcpy(m_pcbMappedLights->m_pLights, m_pLights, sizeof(LIGHT) * m_nLights);
	::memcpy(&m_pcbMappedLights->m_xmf4GlobalAmbient, &m_xmf4GlobalAmbient, sizeof(XMFLOAT4));
	::memcpy(&m_pcbMappedLights->m_nLights, &m_nLights, sizeof(int));
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(LIGHTS) + 255) & ~255); //256의 배수
	m_pd3dcbLights = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbLights->Map(0, NULL, (void**)&m_pcbMappedLights);
}

std::array<const CD3DX12_STATIC_SAMPLER_DESC, 6> GetStaticSamplers()
{
	// Applications usually only need a handful of samplers.  So just define them all up front
	// and keep them available as part of the root signature.  

	const CD3DX12_STATIC_SAMPLER_DESC pointWrap(
		0, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC pointClamp(
		1, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_POINT, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearWrap(
		2, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC linearClamp(
		3, // shaderRegister
		D3D12_FILTER_MIN_MAG_MIP_LINEAR, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP); // addressW

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
		4, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_WRAP,  // addressW
		0.0f,                             // mipLODBias
		8);                               // maxAnisotropy

	const CD3DX12_STATIC_SAMPLER_DESC anisotropicClamp(
		5, // shaderRegister
		D3D12_FILTER_ANISOTROPIC, // filter
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressU
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressV
		D3D12_TEXTURE_ADDRESS_MODE_CLAMP,  // addressW
		0.0f,                              // mipLODBias
		8);                                // maxAnisotropy

	return { pointWrap, pointClamp, linearWrap, linearClamp, anisotropicWrap, anisotropicClamp };
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice) {

	////텍스쳐 루트시그너쳐 설정
	D3D12_DESCRIPTOR_RANGE descRange[1];
	descRange[0].NumDescriptors = 1;
	descRange[0].BaseShaderRegister = 0;
	descRange[0].RegisterSpace = 0;
	descRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	descRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[4];
	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 32;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[2].Descriptor.ShaderRegister = 4; //Lights
	pd3dRootParameters[2].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;


	pd3dRootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	pd3dRootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	pd3dRootParameters[3].DescriptorTable.pDescriptorRanges = &descRange[0];
	pd3dRootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;





	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
		D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS;

	//STATIC_SMAPLER_DESC를 사용해서 만들어야함
	//
	auto staticSamplers = GetStaticSamplers();





	//매개변수가 없는 루트 시그너쳐를 생성한다.
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = staticSamplers.size();
	d3dRootSignatureDesc.pStaticSamplers = staticSamplers.data();
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	::D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)&pd3dGraphicsRootSignature);

	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return pd3dGraphicsRootSignature;


}

void CScene::AnimateObjects(float fTimeElapsed) {


	if (m_pPlayer->wheelanimation) {
		m_pPlayer->UpdateWheel();
	}
	m_pPlayer->Animate(fTimeElapsed);

	for (BillBoard* pBill : m_BillBoardList) {
		if (pBill->m_bActive)
			pBill->Animate(fTimeElapsed);
	}

	for (BillBoard* pBill : m_SubBillBoardList) {
		if (pBill->m_bActive)
			pBill->Animate(fTimeElapsed);
	}


	////상대편 총알
	//for (int id = 0; id < MAX_USER; ++id) {
	//	for (int i = 0; i < BULLETS; ++i) {
	//		if (AllBullets[id][i].m_bActive) {

	//			AllBullets[id][i].Animate(fTimeElapsed);

	//		}

	//	}
	//}

	//BulletToTank();
	PlayerToObject();
	BulletToObject();
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT  nMessageID, WPARAM wParam, LPARAM lParam) {
	return false;

}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) {
	return false;
}

bool CScene::ProcessInput() {
	return false;
}

void CScene::PrepareRender(ID3D12GraphicsCommandList* pd3dcommandList) {

	//그래픽 루트 시그너쳐를 설정한다.
	pd3dcommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	//파이프라인 상태를 설정한다.
	pd3dcommandList->SetPipelineState(m_pd3dPipelineState);
	//프리미티 토폴로지(삼각형 리스트)를 설정한다.
	pd3dcommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamrea) {


	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamrea->SetViewportsAndScissorRects(pd3dCommandList);
	if (pCamrea) pCamrea->UpdateShaderVariavles(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	D3D12_GPU_VIRTUAL_ADDRESS d3dcbLightsGpuVirtualAddress = m_pd3dcbLights->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(2, d3dcbLightsGpuVirtualAddress); //Lights



	//텍스쳐 전달
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);
	pd3dCommandList->SetDescriptorHeaps(1, &m_MeshManager->pSrvDescriptorHeap);

	CD3DX12_GPU_DESCRIPTOR_HANDLE tex(m_MeshManager->pSrvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());




	//샘플러 설정
	//pd3dCommandList->SetGraphicsRootDescriptorTable(4, m_MeshManager->pSamplerDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
	//씬을 렌더링하는 것은 씬을 구성하는 게임 객체(셰이더를 포함하는 객체)들을 렌더링하는 것이다.

	m_pCllluminatedShader->OnPrepareRender(pd3dCommandList, 1);
	for (CGameObject* object : CGameObjects) {
		object->UpdateAllTansform();
		object->Render(pd3dCommandList, pCamrea);
		m_pCllluminatedShader->OnPrepareRender(pd3dCommandList, 0);
	}
	for (CGameObject* object : m_CGameBackGround) {
		object->UpdateAllTansform();
		object->Render(pd3dCommandList, pCamrea);

	}

	for (BillBoard* pBill : m_BillBoardList) {
		if (pBill->m_bActive) {
			pBill->Update(pCamrea->GetPosition(), XMFLOAT3(0.0f, 1.0f, 0.0f));
			pBill->Render(pd3dCommandList, pCamrea);
		}
	}
	for (BillBoard* pBill : m_SubBillBoardList) {
		if (pBill->m_bActive) {
			pBill->Update(pCamrea->GetPosition(), XMFLOAT3(0.0f, 1.0f, 0.0f));
			pBill->Render(pd3dCommandList, pCamrea);
		}
	}


	m_pCllluminatedShader->OnPrepareRender(pd3dCommandList, 0);
	for (CTank* object : CTankObjects) {
		if (object->m_bActive) {
			object->UpdateAllTansform();
			object->Render(pd3dCommandList, pCamrea);
		}
	}

	//상대편 총알
	for (int id = 0; id < MAX_USER; ++id) {
		for (int i = 0; i < BULLETS; ++i) {
			if (AllBullets[id][i].m_bActive) {

				AllBullets[id][i].UpdateAllTansform();
				AllBullets[id][i].Render(pd3dCommandList, pCamrea);

			}

		}
	}

	for (int id = 0; id < MAX_USER; ++id) {
		for (int i = 0; i < MINES; ++i) {

			if (AllMines[id][i].m_bActive) {
				AllMines[id][i].UpdateAllTansform();
				AllMines[id][i].Render(pd3dCommandList, pCamrea);

			}
		}
	}

}

CGameObject* CScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera)
{
	if (!pCamera) return(NULL);
	XMFLOAT4X4 xmf4x4View = pCamera->GetViewMatrix();
	XMFLOAT4X4 xmf4x4Projection = pCamera->GetProjectionMatrix();
	D3D12_VIEWPORT d3dViewport = pCamera->GetViewport();
	XMFLOAT3 xmf3PickPosition;

	/*화면 좌표계의 점 (xClient, yClient)를 화면 좌표 변환의 역변환과 투영 변환의 역변환을 한다. 그 결과는 카메라
	좌표계의 점이다. 투영 평면이 카메라에서 z-축으로 거리가 1이므로 z-좌표는 1로 설정한다.*/

	xmf3PickPosition.x = (((2.0f * xClient) / d3dViewport.Width) - 1) / xmf4x4Projection._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / d3dViewport.Height) - 1) / xmf4x4Projection._22;
	xmf3PickPosition.z = 1.0f;
	int nIntersected = 0;
	float fHitDistance = FLT_MAX, fNearestHitDistance = FLT_MAX;
	CGameObject* pIntersectedObject = NULL, * pNearestObject = NULL;

	//셰이더의 모든 게임 객체들에 대한 마우스 픽킹을 수행하여 카메라와 가장 가까운 게임 객체를 구한다. 

	for (CGameObject* object : CGameObjects) {

		object->UpdateAllTansform();

		nIntersected = object->PickObjectByRayIntersection(xmf3PickPosition, xmf4x4View, &fHitDistance);

		pIntersectedObject = object;

		if (pIntersectedObject && (fHitDistance < fNearestHitDistance))
		{
			fNearestHitDistance = fHitDistance;
			pNearestObject = pIntersectedObject;
		}

	}

	return(pNearestObject);
}

void CScene::RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{

	m_pBoundingBoxShader->OnPrepareRender(pd3dCommandList, 1);

	for (CGameObject* object : CGameObjects) {
		if (object->m_bActive) {
			object->UpdateAllTansform();
			object->RenderBoundingBox(pd3dCommandList, pCamera);
		}

	}
	for (CTank* object : CTankObjects) {
		if (object->m_bActive) {
			object->UpdateAllTansform();
			object->RenderBoundingBox(pd3dCommandList, pCamera);
		}
	}
	m_pPlayer->OnPrepareRender();
	m_pPlayer->m_pMesh->RenderBoundingBox(pd3dCommandList, pCamera);


	for (int i = 0; i < BULLETS; ++i) {
		if (m_pPlayer->m_ppBullets[i]->m_bActive) {
			m_pPlayer->m_ppBullets[i]->UpdateAllTansform();
			m_pPlayer->m_ppBullets[i]->RenderBoundingBox(pd3dCommandList, pCamera);
		}
	}
	for (int i = 0; i < MINES; ++i) {
		if (m_pPlayer->m_pMine[i]->m_bActive) {
			m_pPlayer->m_pMine[i]->UpdateAllTansform();
			m_pPlayer->m_pMine[i]->RenderBoundingBox(pd3dCommandList, pCamera);
		}
	}

	for (int id = 0; id < MAX_USER; ++id) {

		for (int i = 0; i < MINES; ++i) {
			if (AllMines[id][i].m_bActive) {
				AllMines[id][i].UpdateAllTansform();
				AllMines[id][i].RenderBoundingBox(pd3dCommandList, pCamera);
			}
		}
	}


}

void CScene::BulletToTank()
{
	for (int i = 0; i < BULLETS; ++i) {
		if (m_pPlayer->m_ppBullets[i]->m_bActive) {
			for (CTank* ctank : CTankObjects) {
				if (ctank->m_bActive) {
					if (ctank->m_BoundingBox.Intersects(m_pPlayer->m_ppBullets[i]->m_BoundingBox) || ctank->TopBoundingBox.Intersects(m_pPlayer->m_ppBullets[i]->m_BoundingBox)) {
						ctank->m_bActive = false;
						float t_pos = rand() / float(RAND_MAX);
						for (BillBoard* pBill : m_BillBoardList) {
							if (!pBill->m_bActive) {
								pBill->m_bActive = true;
								pBill->SetPosition(ctank->GetPosition());
								break;
							}
						}
						t_pos = rand() / float(RAND_MAX);
						for (BillBoard* pBill : m_SubBillBoardList) {
							if (!pBill->m_bActive) {
								pBill->m_bActive = true;
								pBill->SetPosition(Vector3::Add(ctank->GetPosition(), XMFLOAT3(t_pos, t_pos, 0)));
								break;
							}
						}
						t_pos = rand() / float(RAND_MAX);
						for (BillBoard* pBill : m_SubBillBoardList) {
							if (!pBill->m_bActive) {
								pBill->m_bActive = true;
								pBill->SetPosition(Vector3::Add(ctank->GetPosition(), XMFLOAT3(t_pos, -t_pos, 0)));
								break;
							}
						}
						t_pos = rand() / float(RAND_MAX);
						for (BillBoard* pBill : m_SubBillBoardList) {
							if (!pBill->m_bActive) {
								pBill->m_bActive = true;
								pBill->SetPosition(Vector3::Add(ctank->GetPosition(), XMFLOAT3(-t_pos, t_pos, 0)));

								break;
							}
						}
						t_pos = rand() / float(RAND_MAX);
						for (BillBoard* pBill : m_SubBillBoardList) {
							if (!pBill->m_bActive) {
								pBill->m_bActive = true;
								pBill->SetPosition(Vector3::Add(ctank->GetPosition(), XMFLOAT3(-t_pos, -t_pos, 0)));

								break;
							}
						}
						m_pPlayer->m_Score++;
					}
				}
			}
		}
	}

}

bool CScene::PlayerToObject() {

	m_pPlayer->UpdateBoundingBox();
	for (int i = 0; i < CGameObjects.size(); ++i) {

		if (m_pPlayer->m_BoundingBox.Intersects(CGameObjects[i]->m_BoundingBox))
			return true;
	}

	return false;

}

void CScene::BulletToObject() {

	for (int i = 0; i < BULLETS; ++i) {
		if (m_pPlayer->m_ppBullets[i]->m_bActive) {
			for (CGameObject* object : CGameObjects) {
				if (object->m_bActive) {
					if (object->m_BoundingBox.Intersects(m_pPlayer->m_ppBullets[i]->m_BoundingBox)) {
						m_pPlayer->m_ppBullets[i]->m_bActive = false;
						m_pPlayer->m_ppBullets[i]->Reset();
					}
				}
			}
		}
	}
}

void CScene::InterporationTank(std::array<int, MAX_USER>& EveryTick, std::deque<SESSION>(&UserPosStore)[MAX_USER], std::array<SESSION, MAX_USER>& Players)
{

	for (int id = 0; id < MAX_USER; ++id) {

		//유효아이디만
		if (Players[id].id < 0) {
			continue;
		}
		if (UserPosStore[id].size() == 0) {
			//선형 보간 의미 없음
			continue;
		}


		if (UserPosStore[id].size() > 1) {

			if (EveryTick[id] > SERVERANIMATIONTICK) {
				//앞의 위치 다 지워버림
				UserPosStore[id].pop_front();
				EveryTick[id] = 0;
				continue;
			}

			SESSION pre = UserPosStore[id][0];
			SESSION now = UserPosStore[id][1];

			////같은 방향 같은 위치일때
			if (pre.status.topDir.x == now.status.topDir.x &&
				pre.status.topDir.y == now.status.topDir.y &&
				pre.status.topDir.z == now.status.topDir.z &&

				pre.status.bottomDir.x == now.status.bottomDir.x &&
				pre.status.bottomDir.y == now.status.bottomDir.y &&
				pre.status.bottomDir.z == now.status.bottomDir.z &&

				pre.status.pos.x == now.status.pos.x &&
				pre.status.pos.y == now.status.pos.y &&
				pre.status.pos.z == now.status.pos.z) {

				UserPosStore[id].pop_front();
				EveryTick[id] = 0;
				continue;
			}


			float ftick = float(EveryTick[id]) / SERVERANIMATIONTICK;

			XMFLOAT3 interBottomDir = Vector3::Normalize(Vector3::Add(Vector3::ScalarProduct(pre.status.bottomDir, 1.0f - ftick, false), Vector3::ScalarProduct(now.status.bottomDir, ftick, false)));
			XMFLOAT3 intertopDir = Vector3::Normalize(Vector3::Add(Vector3::ScalarProduct(pre.status.topDir, 1.0f - ftick, false), Vector3::ScalarProduct(now.status.topDir, ftick, false)));

			XMFLOAT3 interpos = Vector3::Add(pre.status.pos, Vector3::ScalarProduct(Vector3::Subtract(now.status.pos, pre.status.pos), ftick, false));

			if (EveryTick[id] % 10 == 0 && id == 0) {


				//OutputDebugStringA(std::to_string(id).c_str());
				//OutputDebugStringA("의 ftick은	");
				//OutputDebugStringA(std::to_string(EveryTick[id]).c_str());
				//string temp = "0번째 값 위치";
				//temp += std::to_string(pre.status.pos.x) + " " + std::to_string(pre.status.pos.y) + " " + std::to_string(pre.status.pos.z);
				//temp += "1번째 값 위치";
				//temp += std::to_string(now.status.pos.x) + " " + std::to_string(now.status.pos.y) + " " + std::to_string(now.status.pos.z);


				////string temp = "위쪽";
				////temp += std::to_string(intertopDir.x) + " " + std::to_string(intertopDir.y) + " " + std::to_string(intertopDir.z);
				////temp += "          아래";
				////temp += std::to_string(interBottomDir.x) + " " + std::to_string(interBottomDir.y) + " " + std::to_string(interBottomDir.z);
				//temp += "          위치";
				//temp += std::to_string(interpos.x) + " " + std::to_string(interpos.y) + " " + std::to_string(interpos.z)+"\n";
				//OutputDebugStringA(temp.c_str());

			}


			XMFLOAT4X4 TempMatrix;
			TempMatrix._11 = intertopDir.x;
			TempMatrix._12 = intertopDir.y;
			TempMatrix._13 = intertopDir.z;
			TempMatrix._14 = 0.0f;
			TempMatrix._21 = 0;
			TempMatrix._22 = 1.0f;
			TempMatrix._23 = 0;
			TempMatrix._24 = 0.0f;
			XMFLOAT3 LookVector = Vector3::CrossProduct(intertopDir, XMFLOAT3(0, 1, 0));
			TempMatrix._31 = LookVector.x;
			TempMatrix._32 = LookVector.y;
			TempMatrix._33 = LookVector.z;
			TempMatrix._34 = 0.0f;
			TempMatrix._41 = interpos.x;
			TempMatrix._42 = interpos.y;
			TempMatrix._43 = interpos.z;
			TempMatrix._44 = 1.0f;

			//top Info
			CTankObjects[now.id]->TopTransform = TempMatrix;

			//top Info
			TempMatrix._11 = interBottomDir.x;
			TempMatrix._12 = interBottomDir.y;
			TempMatrix._13 = interBottomDir.z;
			LookVector = Vector3::CrossProduct(interBottomDir, XMFLOAT3(0, 1, 0));
			TempMatrix._31 = LookVector.x;
			TempMatrix._32 = LookVector.y;
			TempMatrix._33 = LookVector.z;

			CTankObjects[now.id]->BottomTransform = TempMatrix;

			XMFLOAT3 SubtractPos = Vector3::Subtract(now.status.pos, pre.status.pos);
			float lenth = SubtractPos.x * SubtractPos.x + SubtractPos.y * SubtractPos.y + SubtractPos.z * SubtractPos.z;

			XMFLOAT3 v1(Vector3::CrossProduct(now.status.get_top_dir(), XMFLOAT3(0, 1, 0)));
			XMFLOAT3 v2(SubtractPos);
			v1 = Vector3::Normalize(v1);
			v2 = Vector3::Normalize(v2);
			float dotProduct = Vector3::DotProduct(v1, v2);//물체의 방향과 이동방향을 내적..

			float v1Size = VectorSize(v1);//사이각을 구하기 위한 벡터들의 크기구하기
			float v2Size = VectorSize(v2);
			float betweenangle = acosf(dotProduct / (v1Size * v2Size));//사이각 구하기
	
			float degree = XMConvertToDegrees(betweenangle);
	

			XMMATRIX xmmtxRotate;
			float scales = 100;
			if (degree < 91 && degree > -1) {
				xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(lenth * scales));
			}
			else {
				xmmtxRotate = XMMatrixRotationX(XMConvertToRadians(-(lenth * scales)));
			}




			for (int i = 0; i < 4; ++i) {
				*CTankObjects[now.id]->wheel[i] = Matrix4x4::Multiply(xmmtxRotate, *CTankObjects[now.id]->wheel[i]);
			}


			CTankObjects[now.id]->SetPosition(interpos);
			CTankObjects[now.id]->UpdateBoundingBox();
			EveryTick[id]++;

		}
		else {

			XMFLOAT4X4 TempMatrix;
			TempMatrix._11 = UserPosStore[id][0].status.topDir.x;
			TempMatrix._12 = UserPosStore[id][0].status.topDir.y;
			TempMatrix._13 = UserPosStore[id][0].status.topDir.z;
			TempMatrix._14 = 0.0f;
			TempMatrix._21 = 0;
			TempMatrix._22 = 1.0f;
			TempMatrix._23 = 0;
			TempMatrix._24 = 0.0f;
			XMFLOAT3 LookVector = Vector3::CrossProduct(UserPosStore[id][0].status.topDir, XMFLOAT3(0, 1, 0));
			TempMatrix._31 = LookVector.x;
			TempMatrix._32 = LookVector.y;
			TempMatrix._33 = LookVector.z;
			TempMatrix._34 = 0.0f;
			TempMatrix._41 = UserPosStore[id][0].status.pos.x;
			TempMatrix._42 = UserPosStore[id][0].status.pos.y;
			TempMatrix._43 = UserPosStore[id][0].status.pos.z;
			TempMatrix._44 = 1.0f;

			//top Info
			CTankObjects[id]->TopTransform = TempMatrix;

			//top Info
			TempMatrix._11 = UserPosStore[id][0].status.bottomDir.x;
			TempMatrix._12 = UserPosStore[id][0].status.bottomDir.y;
			TempMatrix._13 = UserPosStore[id][0].status.bottomDir.z;
			LookVector = Vector3::CrossProduct(UserPosStore[id][0].status.bottomDir, XMFLOAT3(0, 1, 0));
			TempMatrix._31 = LookVector.x;
			TempMatrix._32 = LookVector.y;
			TempMatrix._33 = LookVector.z;
			CTankObjects[id]->BottomTransform = TempMatrix;



			CTankObjects[id]->SetPosition(UserPosStore[id][0].status.pos);
			CTankObjects[id]->UpdateBoundingBox();
			EveryTick[id] = 0;
		}
	}
}



void CScene::InitOtherPlayer(std::array<SESSION, MAX_USER>& Players, int id)
{
	//player 정보만



	if (Players[id].id > -1) {
		XMFLOAT4X4 TempMatrix;
		//TempMatrix._11 = Players[id].status.topDir.x;
		//TempMatrix._12 = Players[id].status.topDir.y;
		//TempMatrix._13 = Players[id].status.topDir.z;
		//TempMatrix._14 = 1.0f;
		//TempMatrix._21 = 0;
		//TempMatrix._22 = 1.0f;
		//TempMatrix._23 = 0;
		//TempMatrix._24 = 1.0f;
		//XMFLOAT3 LookVector = Vector3::CrossProduct(Players[id].status.get_top_dir(), XMFLOAT3(0, 1, 0));
		//TempMatrix._31 = LookVector.x;
		//TempMatrix._32 = LookVector.y;
		//TempMatrix._33 = LookVector.z;
		//TempMatrix._34 = 1.0f;
		//TempMatrix._41 = Players[id].status.get_pos().x;
		//TempMatrix._42 = Players[id].status.get_pos().y;
		//TempMatrix._43 = Players[id].status.get_pos().z;
		//TempMatrix._44 = 1.0f;

		////top Info
		//CTankObjects[Players[id].id]->TopTransform = TempMatrix;
		////top Info
		//TempMatrix._11 = Players[id].status.bottomDir.x;
		//TempMatrix._12 = Players[id].status.bottomDir.y;
		//TempMatrix._13 = Players[id].status.bottomDir.z;
		//LookVector = Vector3::CrossProduct(Players[id].status.get_top_dir(), XMFLOAT3(0, 1, 0));
		//TempMatrix._31 = LookVector.x;
		//TempMatrix._32 = LookVector.y;
		//TempMatrix._33 = LookVector.z;
		//CTankObjects[Players[id].id]->BottomTransform = TempMatrix;

		if (Players[id].color == 0) {
			CTankObjects[Players[id].id]->m_TextureAddr = m_MeshManager->BringTexture("Texture/TankBlue.dds");
		}
		else if (Players[id].color == 1) {
			CTankObjects[Players[id].id]->m_TextureAddr = m_MeshManager->BringTexture("Texture/TankRed.dds");
		}
		else if (Players[id].color == 2) {
			CTankObjects[Players[id].id]->m_TextureAddr = m_MeshManager->BringTexture("Texture/TankGreen.dds");
		}
		else if (Players[id].color == 3) {

			CTankObjects[Players[id].id]->m_TextureAddr = m_MeshManager->BringTexture("Texture/TankYellow.dds");
		}


		CTankObjects[Players[id].id]->SetPosition(Players[id].status.get_pos());
		CTankObjects[Players[id].id]->m_bActive = true;
	}

}

void CScene::UpdateOtherPlayer(std::array<SESSION, MAX_USER>& Players, int id) {

	if (Players[id].id > -1) {
		XMFLOAT4X4 TempMatrix;
		TempMatrix._11 = Players[id].status.topDir.x;
		TempMatrix._12 = Players[id].status.topDir.y;
		TempMatrix._13 = Players[id].status.topDir.z;
		TempMatrix._14 = 0.0f;
		TempMatrix._21 = 0;
		TempMatrix._22 = 1.0f;
		TempMatrix._23 = 0;
		TempMatrix._24 = 0.0f;
		XMFLOAT3 LookVector = Vector3::CrossProduct(Players[id].status.get_top_dir(), XMFLOAT3(0, 1, 0));
		TempMatrix._31 = LookVector.x;
		TempMatrix._32 = LookVector.y;
		TempMatrix._33 = LookVector.z;
		TempMatrix._34 = 0.0f;
		TempMatrix._41 = Players[id].status.get_pos().x;
		TempMatrix._42 = Players[id].status.get_pos().y;
		TempMatrix._43 = Players[id].status.get_pos().z;
		TempMatrix._44 = 1.0f;

		//top Info
		CTankObjects[Players[id].id]->TopTransform = TempMatrix;

		//top Info
		TempMatrix._11 = Players[id].status.bottomDir.x;
		TempMatrix._12 = Players[id].status.bottomDir.y;
		TempMatrix._13 = Players[id].status.bottomDir.z;
		LookVector = Vector3::CrossProduct(Players[id].status.get_bottom_dir(), XMFLOAT3(0, 1, 0));
		TempMatrix._31 = LookVector.x;
		TempMatrix._32 = LookVector.y;
		TempMatrix._33 = LookVector.z;

		CTankObjects[Players[id].id]->BottomTransform = TempMatrix;


		CTankObjects[Players[id].id]->SetPosition(Players[id].status.get_pos());
		CTankObjects[Players[id].id]->UpdateBoundingBox();
	}

}


void CScene::ReomvePlayer(int id) {
	CTankObjects[id]->m_bActive = false;
}

void CScene::RefleshBullet(void* packet)
{
	SC_BULLET_PACKET* p = reinterpret_cast<SC_BULLET_PACKET*>(packet);



	for (int i = 0; i < BULLETS; ++i) {

		if (p->in_use_bullets[i]) {
			p->bullets_dir[i] = p->bullets_dir[i];
			AllBullets[p->id][i].m_xmf4x4World._11 = -p->bullets_dir[i].x;
			AllBullets[p->id][i].m_xmf4x4World._12 = -p->bullets_dir[i].y;
			AllBullets[p->id][i].m_xmf4x4World._13 = -p->bullets_dir[i].z;
			AllBullets[p->id][i].m_xmf4x4World._14 = 0;

			AllBullets[p->id][i].m_xmf4x4World._21 = 0;
			AllBullets[p->id][i].m_xmf4x4World._22 = 1.0f;
			AllBullets[p->id][i].m_xmf4x4World._23 = 0;
			AllBullets[p->id][i].m_xmf4x4World._24 = 0.0f;

			XMFLOAT3 LookVector = Vector3::CrossProduct(p->bullets_dir[i], XMFLOAT3(0, 1, 0));
			AllBullets[p->id][i].m_xmf4x4World._31 = LookVector.x;
			AllBullets[p->id][i].m_xmf4x4World._32 = LookVector.y;
			AllBullets[p->id][i].m_xmf4x4World._33 = LookVector.z;
			AllBullets[p->id][i].m_xmf4x4World._34 = 0.0f;
			AllBullets[p->id][i].m_xmf4x4World._41 = p->bullets_pos[i].x;
			AllBullets[p->id][i].m_xmf4x4World._42 = p->bullets_pos[i].y;
			AllBullets[p->id][i].m_xmf4x4World._43 = p->bullets_pos[i].z;
			AllBullets[p->id][i].m_xmf4x4World._44 = 1.0f;


			AllBullets[p->id][i].SetMovingDirection(p->bullets_dir[i]);
			AllBullets[p->id][i].SetFirePosition(p->bullets_pos[i]);
			AllBullets[p->id][i].SetActive(true);
			AllBullets[p->id][i].Animate(0.0f);


		}
		else {
			AllBullets[p->id][i].SetActive(false);
		}
	}

	for (int i = 0; i < MINES; ++i) {

		if (p->in_use_mines[i]) {

			AllMines[p->id][i].m_xmf4x4World._11 = 1.0f;
			AllMines[p->id][i].m_xmf4x4World._12 = 0.0f;
			AllMines[p->id][i].m_xmf4x4World._13 = 0.0f;
			AllMines[p->id][i].m_xmf4x4World._14 = 0.0f;

			AllMines[p->id][i].m_xmf4x4World._21 = 0.0f;
			AllMines[p->id][i].m_xmf4x4World._22 = 1.0f;
			AllMines[p->id][i].m_xmf4x4World._23 = 0.0f;
			AllMines[p->id][i].m_xmf4x4World._24 = 0.0f;
			XMFLOAT3 LookVector = Vector3::CrossProduct(XMFLOAT3(1, 0, 0), XMFLOAT3(0, 1, 0));
			AllMines[p->id][i].m_xmf4x4World._31 = LookVector.x;
			AllMines[p->id][i].m_xmf4x4World._32 = LookVector.y;
			AllMines[p->id][i].m_xmf4x4World._33 = LookVector.z;
			AllMines[p->id][i].m_xmf4x4World._34 = 0.0f;

			AllMines[p->id][i].m_xmf4x4World._41 = p->mines_pos[i].x;
			AllMines[p->id][i].m_xmf4x4World._42 = p->mines_pos[i].y;
			AllMines[p->id][i].m_xmf4x4World._43 = p->mines_pos[i].z;
			AllMines[p->id][i].m_xmf4x4World._44 = 1.0f;


			AllMines[p->id][i].m_bActive = true;
			AllMines[p->id][i].UpdateBoundingBox();
		}
		else {
			AllMines[p->id][i].m_bActive = false;
		}
	}

}
