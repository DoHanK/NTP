#include "GameObject.h"
#include "Shader.h"
#include "Camera.h"
#include "Mesh.h"

CGameObject::CGameObject() {

	XMStoreFloat4x4(&m_xmf4x4World, XMMatrixIdentity());

}

CGameObject::~CGameObject() {


}

void CGameObject::SetShader(CShader* pShader) {


}

void CGameObject::SetMesh(CSumMesh* pMesh) {
	if (m_pMesh) {
		m_pMesh = nullptr;
	}
	m_pMesh = pMesh;
	if(pMesh->m_pMesh)
		m_BoundingBox = pMesh->m_pMesh->m_xmBoundingBox;

}

void CGameObject::ReleaseUploadBuffers() {
	//정점 버퍼를 위한 업로드 버퍼를 소멸 시킨다.


}

void CGameObject::Animate(float fTimeEelapsed) {
	if (m_fMovingSpeed != 0.0f) Move(m_xmf3MovingDirection, m_fMovingSpeed * fTimeEelapsed);
	UpdateBoundingBox();
}

void CGameObject::Move(XMFLOAT3& vDirection, float fSpeed)
{
	SetPosition(m_xmf4x4World._41 + vDirection.x * fSpeed, m_xmf4x4World._42 + vDirection.y * fSpeed, m_xmf4x4World._43 + vDirection.z * fSpeed);
}

void CGameObject::OnPrePareRender() {

}

void CGameObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera) {
	//게임 객체가 카메라에 보이면 렌더링한다. 
	if(m_TextureAddr)
		pd3dCommandList->SetGraphicsRootDescriptorTable(3, *m_TextureAddr);

	if(m_pMesh)
		m_pMesh->Render(pd3dCommandList,pCamera);


}
void CGameObject::UpdateAllTansform() {
	
	m_pMesh->UpdateTransform(&m_xmf4x4World);

}

void CGameObject::UpdateAllTansform(vector< pair<string, XMFLOAT4X4>> T) {
	int num = 0;
	m_pMesh->UpdateTransform(&m_xmf4x4World ,T , num);

}

void CGameObject::UpdateBoundingBox()
{
	if (m_pMesh)
	{
		m_pMesh->m_pMesh->m_xmBoundingBox.Transform(m_BoundingBox, XMLoadFloat4x4(&m_xmf4x4World));
	}
}
//각 메쉬들의 행렬 정보 저장
void CGameObject::InitAnimaition()
{
	if (m_pMesh) {
		m_pMesh->CopyTransform(m_xmf4x4Animation);
	}

}

void CGameObject::Rotate(XMFLOAT3* pxmf3Axis, float fAngle) {

	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);

}

//11장 추가

void CGameObject::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) {


}

void CGameObject::ReleaseShaderVariables() {

}

void CGameObject::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) {

	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

	//객체의 월드 변환 행렬을 루트 상수(32 비트 값)을 통하여 세이더 변수로 복사한다.
	pd3dCommandList->SetGraphicsRoot32BitConstants(0, 16, &xmf4x4World, 0);
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
	
}

void CGameObject::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

//게임 객체의 로컬 z-축 벡터를 반환한다.
XMFLOAT3 CGameObject::GetLook()
{
	XMFLOAT3 temp = XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32,
		m_xmf4x4World._33);
	return(Vector3::Normalize(temp));

}

//게임 객체의 로컬 y-축 벡터를 반환한다.
XMFLOAT3 CGameObject::GetUp() {

	XMFLOAT3 temp = XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22,
		m_xmf4x4World._23);
	return(Vector3::Normalize(temp));
}

//게임 객체의 로컬 x-축 벡터를 반환한다.
XMFLOAT3 CGameObject::GetRight()
{

	XMFLOAT3 temp = XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12,
		m_xmf4x4World._13);
	return(Vector3::Normalize(temp));
}
// 게임 객체를 로컬 x - 축 방향으로 이동한다.
void CGameObject::MoveStrafe(float fDistance) {

	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();

	xmf3Position = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Right, fDistance));
	CGameObject::SetPosition(xmf3Position);

}

//게임 객체를 로컬 y-축 방향으로 이동한다. 
void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Up, fDistance));
	CGameObject::SetPosition(xmf3Position);
}

//z축 방향으로 이동한다.
void CGameObject::MoveForward(float fDistance) {

	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Up, fDistance));
	CGameObject::SetPosition(xmf3Position);

}


void CGameObject::Rotate(float fPitch, float fYaw, float fRoll) {

	XMMATRIX  mtxRoatate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));

	m_xmf4x4World = Matrix4x4::Multiply(mtxRoatate, m_xmf4x4World);
}

bool CGameObject::IsVisible(CCamera* pCamera)
{
	return false;
	
}




void CGameObject::GenerateRayForPicking(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, XMFLOAT3* pxmf3PickRayOrigin, XMFLOAT3* pxmf3PickRayDirection)
{
	XMFLOAT4X4 xmf4x4WorldView = Matrix4x4::Multiply(m_xmf4x4World, xmf4x4View);
	XMFLOAT4X4 xmf4x4Inverse = Matrix4x4::Inverse(xmf4x4WorldView);
	XMFLOAT3 xmf3CameraOrigin(0.0f, 0.0f, 0.0f);
	//카메라 좌표계의 원점을 모델 좌표계로 변환한다. 
	*pxmf3PickRayOrigin = Vector3::TransformCoord(xmf3CameraOrigin, xmf4x4Inverse);
	//카메라 좌표계의 점(마우스 좌표를 역변환하여 구한 점)을 모델 좌표계로 변환한다
	*pxmf3PickRayDirection= Vector3::TransformCoord(xmf3PickPosition, xmf4x4Inverse);
	//광선의 방향 벡터를 구한다
	*pxmf3PickRayDirection = Vector3::Normalize(Vector3::Subtract(*pxmf3PickRayDirection, *pxmf3PickRayOrigin));

}

int CGameObject::PickObjectByRayIntersection(XMFLOAT3& xmf3PickPosition, XMFLOAT4X4& xmf4x4View, float* pfHitDistance )
{
	int nIntersected = 0;
	*pfHitDistance = FLT_MAX;

	if (m_pMesh)
	{
		XMFLOAT3 xmf3PickRayOrigin, xmf3PickRayDirection;

		GenerateRayForPicking( xmf3PickPosition, xmf4x4View, &xmf3PickRayOrigin,  &xmf3PickRayDirection);
		//모델 좌표계의 광선을 생성한다. 
		//모델 좌표계의 광선과 메쉬의 교차를 검사한다. 
		nIntersected = m_pMesh->CheckRayIntersection(xmf3PickPosition, xmf4x4View, xmf3PickRayOrigin, xmf3PickRayDirection, pfHitDistance);
	
	}

	return(nIntersected);

}

void CGameObject::RenderBoundingBox(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_pMesh)
		m_pMesh->RenderBoundingBox(pd3dCommandList, pCamera);

}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Mine Object
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CMine::Animate(float fTimeElapsed)
{

}





//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CMissile Object
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

CMissile::CMissile(float fEffectiveRange) {
	m_fBulletEffectiveRange = fEffectiveRange;
}
void CMissile::SetFirePosition(XMFLOAT3 xmf3FirePosition)
{
	m_xmf3FirePosition = xmf3FirePosition;
	SetPosition(xmf3FirePosition);
}

void CMissile::Reset()
{
	m_pLockedObject = NULL;
	m_fElapsedTimeAfterFire = 0;
	m_fMovingDistance = 0;
	m_fRotationAngle = 0.0f;

	m_bActive = false;
}

void CMissile::Animate(float fTimeElapsed)
{
	m_fElapsedTimeAfterFire += fTimeElapsed;

	float fDistance = m_fMovingSpeed * fTimeElapsed;

	if ((m_fElapsedTimeAfterFire > m_fLockingDelayTime) && m_pLockedObject)
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMVECTOR xmvPosition = XMLoadFloat3(&xmf3Position);

		XMFLOAT3 xmf3LockedObjectPosition = m_pLockedObject->GetPosition();
		XMVECTOR xmvLockedObjectPosition = XMLoadFloat3(&xmf3LockedObjectPosition);
		XMVECTOR xmvToLockedObject = xmvLockedObjectPosition - xmvPosition;
		xmvToLockedObject = XMVector3Normalize(xmvToLockedObject);

		XMVECTOR xmvMovingDirection = XMLoadFloat3(&m_xmf3MovingDirection);
		xmvMovingDirection = XMVector3Normalize(XMVectorLerp(xmvMovingDirection, xmvToLockedObject, 0.25f));
		XMStoreFloat3(&m_xmf3MovingDirection, xmvMovingDirection);
	}
#ifdef _WITH_VECTOR_OPERATION
	XMFLOAT3 xmf3Position = GetPosition();

	m_fRotationAngle += m_fRotationSpeed * fElapsedTime;
	if (m_fRotationAngle > 360.0f) m_fRotationAngle = m_fRotationAngle - 360.0f;

	XMFLOAT4X4 mtxRotate1 = Matrix4x4::RotationYawPitchRoll(0.0f, m_fRotationAngle, 0.0f);

	XMFLOAT3 xmf3RotationAxis = Vector3::CrossProduct(m_xmf3RotationAxis, m_xmf3MovingDirection, true);
	float fDotProduct = Vector3::DotProduct(m_xmf3RotationAxis, m_xmf3MovingDirection);
	float fRotationAngle = ::IsEqual(fDotProduct, 1.0f) ? 0.0f : (float)XMConvertToDegrees(acos(fDotProduct));
	XMFLOAT4X4 mtxRotate2 = Matrix4x4::RotationAxis(xmf3RotationAxis, fRotationAngle);

	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate1, mtxRotate2);

	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(m_xmf3MovingDirection, fDistance, false);
	xmf3Position = Vector3::Add(xmf3Position, xmf3Movement);
	SetPosition(xmf3Position);
#else
	XMMATRIX xmRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(0.0f), XMConvertToRadians(0.0f), XMConvertToRadians(m_fRotationSpeed * fTimeElapsed));
	XMStoreFloat4x4(&m_xmf4x4World, xmRotate * XMLoadFloat4x4(&m_xmf4x4World));
	XMFLOAT3 xmf3Position = GetPosition();
	XMStoreFloat3(&xmf3Position, XMLoadFloat3(&xmf3Position) + (XMLoadFloat3(&m_xmf3MovingDirection) * fDistance));
	m_xmf4x4World._41 = xmf3Position.x; m_xmf4x4World._42 = xmf3Position.y; m_xmf4x4World._43 = xmf3Position.z;
	m_fMovingDistance += fDistance;
#endif

	CGameObject::Animate(fTimeElapsed);

	if ((m_fMovingDistance > m_fBulletEffectiveRange) || (m_fElapsedTimeAfterFire > m_fLockingTime)) Reset();

}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//CTank Object
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
CTank::CTank(CSumMesh* Mesh)
{
	SetMesh(Mesh);
	CSumMesh* findFrame = Mesh->FindFrame("TankFree_Tower");
	TopMesh = findFrame;
	TopBoundingBox = Mesh->m_pMesh->m_xmBoundingBox;
	XMStoreFloat4x4(&TopTransform, XMMatrixIdentity());
	XMStoreFloat4x4(&BottomTransform, XMMatrixIdentity());
	CGameObject::InitAnimaition();
	auto p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Tower"; });
	TopTransform = (p->second);
	UpdateBoundingBox();

}

void CTank::Animate(float fTimeElapsed)
{
	//CGameObject::Animate(fTimeElapsed);
}

void CTank::FindFrameSet()
{
	auto p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Wheel_b_right"; });
	wheel[0] = &(p->second);
	p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Wheel_b_left"; });
	wheel[1] = &(p->second);
	p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Wheel_f_right"; });
	wheel[2] = &(p->second);
	p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Wheel_f_left"; });
	wheel[3] = &(p->second);
	 p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Tower"; });
	Top = &(p->second);
	p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Canon"; });
	Gun = &(p->second);
	p = find_if(m_xmf4x4Animation.begin(), m_xmf4x4Animation.end(), [](pair<string, XMFLOAT4X4> temp) {return temp.first == "TankFree_Green"; });
	Bottom = &(p->second);

	TopMesh = m_pMesh->FindFrame("TankFree_Tower");
	GunMesh = m_pMesh->FindFrame("TankFree_Canon");
	BottomMesh = m_pMesh->FindFrame("TankFree_Green");
	WheelMesh[0] = m_pMesh->FindFrame("TankFree_Wheel_b_right");
	WheelMesh[1] = m_pMesh->FindFrame("TankFree_Wheel_b_left");
	WheelMesh[2] = m_pMesh->FindFrame("TankFree_Wheel_f_right");
	WheelMesh[3] = m_pMesh->FindFrame("TankFree_Wheel_f_left");
}

void CTank::UpdateBoundingBox()
{
	if (m_pMesh) {
		m_pMesh->m_pMesh->m_xmBoundingBox.Transform(m_BoundingBox, XMLoadFloat4x4(&m_xmf4x4World));
		XMStoreFloat4(&m_BoundingBox.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_BoundingBox.Orientation)));
		XMFLOAT4X4 temp = Matrix4x4::Multiply(TopTransform,m_xmf4x4World);


		TopMesh->m_pMesh->m_xmBoundingBox.Transform(TopBoundingBox, XMLoadFloat4x4(&temp));
		XMStoreFloat4(&TopBoundingBox.Orientation, XMQuaternionNormalize(XMLoadFloat4(&TopBoundingBox.Orientation)));
	}

}

void CTank::OnPrepareRender()
{
	//위 아래 정보를 받아올것
	UpdateAllTansform();
}

void CTank::UpdateAllTansform()
{
	BottomMesh->m_xmf4x4World = Matrix4x4::Multiply(*Bottom, BottomTransform);
	WheelMesh[0]->m_xmf4x4World = Matrix4x4::Multiply(*wheel[0], BottomMesh->m_xmf4x4World);
	WheelMesh[1]->m_xmf4x4World = Matrix4x4::Multiply(*wheel[1], BottomMesh->m_xmf4x4World);
	WheelMesh[2]->m_xmf4x4World = Matrix4x4::Multiply(*wheel[2], BottomMesh->m_xmf4x4World);
	WheelMesh[3]->m_xmf4x4World = Matrix4x4::Multiply(*wheel[3], BottomMesh->m_xmf4x4World);
	TopMesh->m_xmf4x4World = Matrix4x4::Multiply(*Top, TopTransform);
	GunMesh->m_xmf4x4World = Matrix4x4::Multiply(*Gun, TopMesh->m_xmf4x4World);
}

void CTank::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();
	CGameObject::Render(pd3dCommandList, pCamera);
}

void BillBoard::Animate(float fTimeElapsed)
{


	m_BillMesh->UpdateUvCoord(UIRect{ m_row * (1.0f / m_rows) , (m_row +1.0f) * (1.0f / m_rows) , m_col * (1.0f / m_cols ), (m_col +1.f) * (1.0f / m_cols)});
	//m_BillMesh->UpdateUvCoord(UIRect{ 1 , 0 , 0, 1});
		//top bottom left right

	Timer += fTimeElapsed;
	if (Timer > SettedTimer) {
		Timer = 0;
	}

	if (Timer == 0) {
		m_col++;
		if (m_col == m_cols) {
			m_row++;
			m_col = 0;
		}
		if (m_row == m_rows) {
			m_row = 0;
			m_bActive = 0;
		}
	}
}

void BillBoard::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	//게임 객체가 카메라에 보이면 렌더링한다. 
	if (m_TextureAddr)
		pd3dCommandList->SetGraphicsRootDescriptorTable(3, *m_TextureAddr);

	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);

	if(m_BillMesh)
		m_BillMesh->Render(pd3dCommandList);
}

void BillBoard::Update(XMFLOAT3 xmf3Target ,XMFLOAT3 xmf3Up)
{
	XMFLOAT3 xmf3Position(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43);
	XMFLOAT4X4 mtxLookAt = Matrix4x4::LookAtLH(xmf3Position, xmf3Target, xmf3Up);
	m_xmf4x4World._11 = mtxLookAt._11; m_xmf4x4World._12 = mtxLookAt._21; m_xmf4x4World._13 = mtxLookAt._31;
	m_xmf4x4World._21 = mtxLookAt._12; m_xmf4x4World._22 = mtxLookAt._22; m_xmf4x4World._23 = mtxLookAt._32;
	m_xmf4x4World._31 = mtxLookAt._13; m_xmf4x4World._32 = mtxLookAt._23; m_xmf4x4World._33 = mtxLookAt._33;


}
