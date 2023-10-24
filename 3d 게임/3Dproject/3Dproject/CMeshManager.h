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
	//������ �ش��ϴ� ���� ����
	list<pair<string,CSumMesh*>> MeshList;

	//�ؽ��Ŀ� ���� ���� ���� // �ش��̸��� ��ũ�������� �ε��� �ּҸ� �˷���
	list< pair<string , CD3DX12_GPU_DESCRIPTOR_HANDLE*>> TextureList;

	ID3D12DescriptorHeap* pSamplerDescriptorHeap = nullptr;

	ID3D12DescriptorHeap* pSrvDescriptorHeap = nullptr;
	//��ũ������ �ڵ� �ּ�
	CD3DX12_GPU_DESCRIPTOR_HANDLE  hGDescriptor;
	CD3DX12_CPU_DESCRIPTOR_HANDLE  hCDescriptor;

	UINT  size = 0;
	//�ؽ��������� ������
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
	//ī���õ� �޽� ����
	UINT counting = 0;

	void CrateSrvDescriptorHeap(ID3D12Device* pd3dDevice , int numDesc);

	void CreateSampleDescriptorHeap(ID3D12Device* pd3dDevice);
	//���� �޽��� ������ �о� ���̰�, �մ� �޽���� �����մ� �޽��� ��ȯ����
	CSumMesh* BringMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dRootSignature, const char* filename);
	CSumMesh* BringMesh(const char* filename);
	
	string ObjectNameFromFile(const char* filename);
	//ó���� ���α׷��� upload�Ҷ� ���� ��ȯ������, �ش� �ؽ����� gpu �ּҸ� ��ȯ ����
	CD3DX12_GPU_DESCRIPTOR_HANDLE* BringTexture(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const char* filename);
	//�߰��� �ʿ��Ҷ� ���ϸ� �Է��ϸ� ��ȯ����
	CD3DX12_GPU_DESCRIPTOR_HANDLE* CMeshManager::BringTexture(const char* filename);

	CD3DX12_GPU_DESCRIPTOR_HANDLE* BringTextureFromMesh(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const char* filename);

	void MakeNaviationMesh(const std::vector<CGameObject>& CGameObject);

};

