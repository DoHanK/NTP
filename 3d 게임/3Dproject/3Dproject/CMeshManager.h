#pragma once
#include "Mesh.h"
#include "GameObject.h"
#include "DDSTextureLoader.h"

class Texture {
public:
	std::string Name;

	std::wstring Filename;

	Microsoft::WRL::ComPtr<ID3D12Resource> Resource = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadHeap = nullptr;

public:
	Texture() {};
	~Texture() {  };
	Texture(const Texture& other) {
		Name = other.Name;
		Filename = other.Filename;
		if (other.Resource)
			Resource = other.Resource;
		if (other.UploadHeap)
			UploadHeap = other.UploadHeap;
	}
	Texture& operator=(const Texture& other) {
		if (this == &other) {
			return *this;
		}

		Name = other.Name;
		Filename = other.Filename;
		if (other.Resource)
			Resource = other.Resource;
		if (other.UploadHeap)
			UploadHeap = other.UploadHeap;
	}

	Texture(Texture&& other) {

		Name = other.Name;
		Filename = other.Filename;

		if (other.Resource)
		{
			Resource = other.Resource;
			other.Resource = nullptr;
		}
		if (other.UploadHeap) {
			UploadHeap = other.UploadHeap;
			other.UploadHeap = nullptr;
		}
	}
	Texture& operator=(Texture&& other) {
		if (this == &other) {
			return *this;
		}
		Name = other.Name;
		Filename = other.Filename;

		if (other.Resource)
		{
			Resource = other.Resource;
			other.Resource = nullptr;
		}
		if (other.UploadHeap) {
			UploadHeap = other.UploadHeap;
			other.UploadHeap = nullptr;
		}
	
	};



};


class CMeshManager{
public:
	CMeshManager(ID3D12Device* pd3dDevice);
	~CMeshManager() {};
	//정점에 해당하는 정보 저장
	list<pair<string,CSumMesh*>> MeshList;

	//텍스쳐에 대한 정보 저장 // 해당이름과 디스크립터힙의 인덱스 주소를 알려줌
	list< pair<string , CD3DX12_GPU_DESCRIPTOR_HANDLE*>> TextureList;

	ID3D12DescriptorHeap* pSamplerDescriptorHeap = nullptr;

	ID3D12DescriptorHeap* pSrvDescriptorHeap = nullptr;
	//디스크립터의 핸들 주소
	CD3DX12_GPU_DESCRIPTOR_HANDLE  hGDescriptor;
	CD3DX12_CPU_DESCRIPTOR_HANDLE  hCDescriptor;

	UINT  size = 0;
	//텍스쳐파일의 오프셋
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//카운팅된 메쉬 정보
	UINT counting = 0;

	void CrateSrvDescriptorHeap(ID3D12Device* pd3dDevice , int numDesc);

	void CreateSampleDescriptorHeap(ID3D12Device* pd3dDevice);
	//없는 메쉬는 파일을 읽어 들이고, 잇는 메쉬라면 갖고잇는 메쉬를 반환해줌
	CSumMesh* BringMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, const char* filename);
	CSumMesh* BringMesh(const char* filename);
	
	string ObjectNameFromFile(const char* filename);
	//처음에 프로그램에 upload할때 쓰면 반환값으로, 해당 텍스쳐의 gpu 주소를 반환 해줌
	CD3DX12_GPU_DESCRIPTOR_HANDLE* BringTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const char* filename);
	//중간에 필요할때 파일명만 입력하면 반환해줌
	CD3DX12_GPU_DESCRIPTOR_HANDLE* CMeshManager::BringTexture(const char* filename);

	CD3DX12_GPU_DESCRIPTOR_HANDLE* BringTextureFromMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const char* filename);

	void MakeNaviationMesh(const std::vector<CGameObject>& CGameObject);

};

