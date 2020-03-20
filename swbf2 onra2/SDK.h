#pragma once
#include "Memory.h"
#include "Classes.h"
#include "D2DOverlay.h"
#pragma warning(disable:4996)

void DumpBoneIDs()
{
	std::ofstream outputFile;
	outputFile.open(R"(C:\Users\Gebruiker\Desktop\Bones.txt)");

	PlayerManager* pPlayerManager = GameContext::Instance()->pPlayerManager;
	ClientPlayer* pLocalPlayer = pPlayerManager->LocalPlayer;
	if (!IsValidPtr(pLocalPlayer))
	{
		outputFile.close();
		return;
	}
		
	ClientSoldier* pLocalSoldier = pLocalPlayer->ControlledControllable;
	if (!IsValidPtr(pLocalSoldier))
	{
		outputFile.close();
		return;
	}
		
	WSClientSoldierEntity* pWSClientSoldierEntity = pLocalSoldier->WSClientSoldierEntity;
	if (!IsValidPtr(pWSClientSoldierEntity))
	{
		outputFile.close();
		return;
	}
		
	ClientBoneCollisionComponent* LocalBoneCollision = pWSClientSoldierEntity->getBoneCollision();
	if (!IsValidPtr(LocalBoneCollision))
	{
		outputFile.close();
		return;
	}
	outputFile << std::hex << std::uppercase << "ClientBoneCollisionComponent 0x" << LocalBoneCollision << std::endl;

	AnimationSkeleton* pAnimationSkelet = LocalBoneCollision->animationSkeleton;
	if (!IsValidPtr(pAnimationSkelet))
	{
		outputFile.close();
		return;
	}

	auto pSkeletonAsset = pAnimationSkelet->skeletonAsset;
	if (!IsValidPtr(pSkeletonAsset))
	{
		outputFile.close();
		return;
	}

	std::cout << pSkeletonAsset->CharacterType;
	/*for (int i = 0; i < pAnimationSkelet->m_BoneCount; i++)
	{
		char* name = pAnimationSkelet->GetBoneNameAt(i);
		std::cout << name << " = " << i << std::endl;
	}*/

	outputFile.close();
}

float Distance3D(D3DXVECTOR3  v1, D3DXVECTOR3 v2)
{
	float x_d = (v2.x - v1.x);
	float y_d = (v2.y - v1.y);
	float z_d = (v2.z - v1.z);
	return sqrt((x_d * x_d) + (y_d * y_d) + (z_d * z_d));
}

bool WorldToScreen(RECT rect, D3DXVECTOR3& vLocVec4, RenderView* pRenderer)
{
	D3DXMATRIXA16 ViewProj = pRenderer->viewProj;
	float mX = (rect.right - rect.left) / 2;
	float mY = (rect.bottom - rect.top) / 2;

	float w =
		ViewProj(0, 3) * vLocVec4.x +
		ViewProj(1, 3) * vLocVec4.y +
		ViewProj(2, 3) * vLocVec4.z +
		ViewProj(3, 3);

	if (w < 0.65f)
	{
		vLocVec4.z = w;
		return false;
	}

	float x =
		ViewProj(0, 0) * vLocVec4.x +
		ViewProj(1, 0) * vLocVec4.y +
		ViewProj(2, 0) * vLocVec4.z +
		ViewProj(3, 0);

	float y =
		ViewProj(0, 1) * vLocVec4.x +
		ViewProj(1, 1) * vLocVec4.y +
		ViewProj(2, 1) * vLocVec4.z +
		ViewProj(3, 1);

	vLocVec4.x = (mX + mX * x / w);
	vLocVec4.y = (mY - mY * y / w);
	vLocVec4.z = w;
	return true;
}

D3DXVECTOR2 CalcAim(D3DXVECTOR3 target, RenderView* pRenderView)
{
	D3DXVECTOR3 player = pRenderView->CameraPos;
	float a = target.x - player.x;
	float b = target.z - player.z;
	float yaw = float(atan(a / b));
	if (b < 0) {
		yaw += float(M_PI);
	}
	if (yaw < 0) {
		yaw = float(M_PI * 2) + yaw;
	}
	yaw = float(M_PI) * 2 - yaw;
	float z = target.y - player.y;
	float c = float(sqrt(a * a + b * b));
	float pitch = float(atan(z / c));
	return { pitch, yaw };
}

void bytePatch(DWORD64 address, std::vector<BYTE> patch)
{
	DWORD oldProtect;
	VirtualProtect(LPVOID(address), patch.size(), PAGE_EXECUTE_READWRITE, &oldProtect);
	memcpy(reinterpret_cast<void*>(address), patch.data(), patch.size());
	VirtualProtect(LPVOID(address), patch.size(), oldProtect, nullptr);
}

void writeNops(DWORD64 address, size_t length)
{
	bytePatch(address, std::vector<BYTE>(length, 0x90));
}

bool setDamage(HANDLE hProc, int val)
{
	BYTE* shellcode = new BYTE[6]{ 0xb9, 0x00, 0x00, 0x00, 0x00, 0x90 };
	memcpy(shellcode + 1, &val, 4);
	SIZE_T bytesWritten;
	WriteProcessMemory(hProc, LPVOID(OFFSET_XDMG), shellcode, 6, &bytesWritten);
	return bytesWritten == 6;
}

void DrawBones(Direct2DOverlay* ov, RenderView* renderView, D3DXCOLOR Chosencolor, ClientBoneCollisionComponent* bone_collision, int Bone1, int Bone2)
{
	if (!IsValidPtr(bone_collision))
		return;

	D3DXVECTOR3 Bone1Vec = bone_collision->GetBonePos(Bone1);
	D3DXVECTOR3 Bone2Vec = bone_collision->GetBonePos(Bone2);

	RECT Overlay = ov->GetOverlayRect();	
	if (WorldToScreen(Overlay, Bone1Vec, renderView) && WorldToScreen(Overlay, Bone2Vec, renderView))
	{
		ov->DrawLine(Bone1Vec.x, Bone1Vec.y, Bone2Vec.x, Bone2Vec.y, 1, Chosencolor.r, Chosencolor.g, Chosencolor.b);
	}
}

void DrawBones(Direct2DOverlay* ov, RenderView* renderView, D3DXCOLOR Chosencolor, ClientBoneCollisionComponent* bone_collision, const char* Bone1, const char* Bone2)
{
	if (!IsValidPtr(bone_collision))
		return;

	RECT Overlay = ov->GetOverlayRect();
	D3DXVECTOR3 Bone1Vec;
	D3DXVECTOR3 Bone2Vec;

	if (!bone_collision->GetBone(Bone1, Bone1Vec))
		return;
	if (bone_collision->GetBone(Bone2, Bone2Vec))
		return;
	if (!WorldToScreen(Overlay, Bone1Vec, renderView))
		return;
	if (!WorldToScreen(Overlay, Bone2Vec, renderView))
		return;
	
	ov->DrawLine(Bone1Vec.x, Bone1Vec.y, Bone2Vec.x, Bone2Vec.y, 1, Chosencolor.r, Chosencolor.g, Chosencolor.b);
}

//ViewAngleStruct* pViewAngleStruct = nullptr;
//
//typedef void* (__fastcall* VIEWANGLEFUNC2)(void* a1, ViewAngleStruct* a2, void* a3);
//VIEWANGLEFUNC2 oViewAngleFunc2 = nullptr;
//
//void* hkViewAngleFunc2(void* a1, ViewAngleStruct* a2, void* a3)
//{
//	pViewAngleStruct = a2;
//	return oViewAngleFunc2(a1, a2, a3);
//}
//
//bool writeViewAngles(float yaw, float pitch)
//{
//	if (IsValidPtr(pViewAngleStruct))
//	{
//		pViewAngleStruct->yaw = yaw;
//		pViewAngleStruct->pitch = pitch;
//		return true;
//	}
//	return false;
//}