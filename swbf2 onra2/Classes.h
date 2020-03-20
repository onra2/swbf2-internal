#pragma once
#include <iostream>
#include <windows.h>
#include <TlHelp32.h>
#include <vector>
#include <d3dx9math.h>
#pragma comment(lib, "d3dx9.lib")
#include <algorithm>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctime>
#include <emmintrin.h>

#ifndef IsValidPtr
#define IsValidPtr(addr) ((DWORD64)(addr) > 0x100 && (DWORD64)(addr) < 0x00007fffffffffff  && !IsBadReadPtr(addr, sizeof(PVOID)))
#endif
#pragma warning(disable:4996)

#define OFFSET_XDMG 0x14187e47b
#define OFFSET_DXRENDERER 0x1446D2318
#define OFFSET_GAMERENDERER 0x1446D2710
#define OFFSET_WORLDRENDERER 0x1445E04F7
#define OFFSET_GAMETIMESETTINGS 0x1441CBD80
#define OFFSET_CAMERAUPDATE 0x1409B32A0
#define OFFSET_MOUSEHOOK 0x141197E80
#define OFFSET_DRAWOUTLINE 0x141E1495C
#define OFFSET_CLIENTGAMECONTEXT 0x1444B0BB8
#define OFFSET_FIRSTTYPEINFO 0x1441DE738
#define OFFSET_VIEWANGLEFUNC 0x1416EFD03
#define OFFSET_MAIN 0x1444753C0
#define OFFSET_SPREADPATCH 0x1416DD149
#define OFFSET_RECOILPATCH 0x1416E0E7F
#define OFFSET_DISPATCHMESSAGE 0x1401EEFA0
#define OFFSET_DEBUGRENDERER 0x1409BC0E8
#define OFFSET_DRAWLINE 0x1414B2A2A
#define OFFSET_DRAWRECT2D 0x1409BC206
#define OFFSET_DRAWTEXT 0x1414F3716
#define OFFSET_AIMER 0x144744D60 //48 8B 0D ? ? ? ? 4C 8B 01 8B 50 38, 0x3, 0x7
#define OFFSET_HELPFULSHIT 0x143fae5c8 //48 8b 05 ? ? ? ? 48 63 58 40 e8 ? ? ? ? 48

struct typeInfoMemberResult {
	void* pVTable;
	const char* name;
	DWORD offset;
};

std::vector<typeInfoMemberResult> typeInfoMemberResults;

template <class T = void*>
T GetClassFromName(void* addr, const char* name, SIZE_T classSize = 0x2000, bool rescan = false) {
	if (!rescan) {
		for (typeInfoMemberResult& result : typeInfoMemberResults) {
			if (result.pVTable == addr) {
				if (result.name == name) {
					return *(T*)((DWORD64)addr + result.offset);
				}
			}
		}
	}

	const byte INSTR_LEA = 0x48;
	const byte INSTR_RET = 0xc3;
	const byte INSTR_JMP = 0xe9;
	const DWORD64 BASE_ADDRESS = 0x140000000;
	const DWORD64 MAX_ADDRESS = 0x14fffffff;

	DWORD offset = 0;
	DWORD lastOffset = 0;
	while (offset < classSize) {
		offset += 8;
		if (!IsValidPtr((void*)((DWORD64)addr + offset))) continue;
		void* czech = *(void**)((DWORD64)addr + offset);
		if (!IsValidPtr(czech)) continue;
		if (!IsValidPtr(*(void**)czech)) continue; // vtable
		if (!IsValidPtr(**(void***)czech)) continue; // virtual 1;
		void* pGetType = **(DWORD64***)czech;
		if ((DWORD64)pGetType < BASE_ADDRESS || (DWORD64)pGetType > MAX_ADDRESS) continue;

		if (*(byte*)pGetType == INSTR_JMP || *(byte*)pGetType == INSTR_LEA) {
			void* pTypeInfo = nullptr;
			if (*(byte*)pGetType == INSTR_JMP) {
				std::cout << std::hex << "rel:\t" << *(int32_t*)((DWORD64)pGetType + 1) << "\tRIP:\t" << (DWORD64)pGetType + 5 << std::endl;
				pGetType = (void*)(*(int32_t*)((DWORD64)pGetType + 1) + (DWORD64)pGetType + 5);
			}
			if (*(byte*)pGetType == INSTR_LEA) {
				if (*(byte*)((DWORD64)pGetType + 7) != INSTR_RET) continue;
				pTypeInfo = (void*)(*(int32_t*)((DWORD64)pGetType + 3) + (DWORD64)pGetType + 7);
			}
			else continue;
			if (!IsValidPtr(pTypeInfo)) continue;
			void* pMemberInfo = *(void**)pTypeInfo;
			if (!IsValidPtr(pMemberInfo)) continue;
			char* m_name = *(char**)pMemberInfo;
			if (!IsValidPtr(m_name)) continue;
			if ((DWORD64)pTypeInfo > BASE_ADDRESS && (DWORD64)pTypeInfo < MAX_ADDRESS) {
				if (strcmp(m_name, name) == 0) {
					typeInfoMemberResult result;
					result.name = name;
					result.offset = offset;
					result.pVTable = addr;
					typeInfoMemberResults.push_back(result);
					return *(T*)((DWORD64)addr + offset);
				}

				lastOffset = offset;
			}
		}
	}
	return nullptr;
}

class SoldierCustomizatoinKitAsset
{
public:
	char pad_0000[24]; //0x0000
	char* PlayerName; //0x0018
	char pad_0020[48]; //0x0020
	int64_t AnimationFitGameplay; //0x0050
	char pad_0058[8]; //0x0058
	int64_t AnimationFitCharacterChoose; //0x0060
	int64_t Skin; //0x0068
	char pad_0070[24]; //0x0070
}; //Size: 0x0088

class SoldierClasses
{
public:
	class SoldierCustomizatoinKitAsset* Assault; //0x0000
	class SoldierCustomizatoinKitAsset* Heavy; //0x0008
	class SoldierCustomizatoinKitAsset* Officer; //0x0010
	class SoldierCustomizatoinKitAsset* Specialist; //0x0018
	char pad_0020[40]; //0x0020
}; //Size: 0x0048

class Soldiers
{
public:
	char pad_0000[32]; //0x0000
	class SoldierClasses* SoldierClasses; //0x0020
	char pad_0028[32]; //0x0028
}; //Size: 0x0048

class Heroes
{
public:
	char pad_0000[64]; //0x0000
	class SoldierCustomizatoinKitAsset* HeroArray[4]; //0x0040
	char pad_0060[164]; //0x0060
}; //Size: 0x0104

class Reinforcements
{
public:
	char pad_0000[64]; //0x0000
	class SoldierCustomizatoinKitAsset* Aerial; //0x0040
	class SoldierCustomizatoinKitAsset* Enforcer; //0x0048
	char pad_0050[4]; //0x0050
}; //Size: 0x0054

class TeamData
{
public:
	char pad_0000[72]; //0x0000
	class Soldiers* Soldiers; //0x0048
	char pad_0050[8]; //0x0050
	class Heroes* Heroes; //0x0058
	class SoldierCustomizatoinKitAsset* Reinforcements; //0x0060
	char pad_0068[32]; //0x0068
}; //Size: 0x0088

class TeamEntity
{
public:
	char pad_0000[40]; //0x0000
	class TeamData* TeamData; //0x0028
	char pad_0030[24]; //0x0030
}; //Size: 0x0048

class Teams
{
public:
	char pad_0000[16]; //0x0000
	class TeamEntity* TeamLight; //0x0010
	class TeamEntity* TeamDark; //0x0018
	char pad_0020[40]; //0x0020
}; //Size: 0x0048

class Level
{
public:
	char pad_0000[40]; //0x0000
	class Teams* Teams; //0x0028
	char pad_0030[24]; //0x0030
}; //Size: 0x0048

class ClientSoldierHealthComponent
{
public:
	char pad_0000[32]; //0x0000
	float Health; //0x0020
	float m_flMaxHealth; //0x0024
	char pad_0024[20]; //0x0024
	//float MaxHealth; //0x0038
};

class ClientSoldierPrediction
{
public:
	char pad_0000[32]; //0x0000
	D3DXVECTOR3 Location; //0x0020
	char pad_0001[4];
	D3DXVECTOR3 Velocity; //0x0030
};

class ClientAimEntity
{
public:
	char pad_0000[224]; //0x0000
	D3DXVECTOR4 CachedRayCast; //0x00E0
};

class FiringFunctionData
{
public:
	char pad_0000[264]; //0x0000
	float BulletSpeed; //0x0108
	char pad_010C[68]; //0x010C
	uint32_t BulletsPerShot; //0x0150
	char pad_0154[756]; //0x0154
}; //Size: 0x0448

class WeaponFiring
{
public:
	char pad_0000[24]; //0x0000
	class FiringFunctionData* firingFunctionData; //0x0018
	char pad_0020[40]; //0x0020
}; //Size: 0x0048

class SoldierWeaponData
{
public:
	char pad_0000[152]; //0x0000
	class WeaponFiring* weaponFiring; //0x0098
	char pad_00A0[16]; //0x00A0
}; //Size: 0x00B0

class ClientSoldierWeapon
{
public:
	char pad_0000[56]; //0x0000
	class SoldierWeaponData* soldierWeaponData; //0x0038
}; //Size: 0x0038

enum HumanBones
{
	Head = 48,
	Neck = 46,
	Spine = 5,
	Spine1 = 6,
	Spine2 = 7,
	LeftShoulder = 8,
	LeftElbowRoll = 13,
	RightShoulder = 144,
	RightElbowRoll = 149,
	LeftHand = 17,
	RightHand = 153,
	RightKneeRoll = 235,
	LeftKneeRoll = 223,
	RightFoot = 228,
	LeftFoot = 216
};

class UpdatePoseResultData
{
public:
	class QuatTransform
	{
	public:
		D3DXVECTOR4 m_TransAndScale; //0x0000 
		D3DXVECTOR4 m_Rotation; //0x0010 
	};//Size=0x0020

	QuatTransform* m_LocalTransforms; //0x0000 
	QuatTransform* m_WorldTransforms; //0x0008 
	QuatTransform* m_Unk; //0x0010 
	QuatTransform* m_Unk1; //0x0018 
	QuatTransform* m_Unk2; //0x0020 
	QuatTransform* m_ActiveWorldTransforms; //0x0028 
	QuatTransform* m_ActiveLocalTransforms; //0x0030 
	__int32 m_Slot; //0x0038 
	__int32 m_ReaderIndex; //0x003C 
	unsigned char m_ValidTransforms; //0x0040 
	unsigned char m_PoseUpdateNeeded; //0x0041 
	unsigned char m_PoseNeeded; //0x0042 
};

class SkeletonAsset
{
public:
	char pad_0000[24]; //0x0000
	char* CharacterType; //0x0018
	char** BoneNames; //0x0020
}; //Size: 0x0048

class AnimationSkeleton
{
public:
	char pad_0000[8]; //0x0000
	SkeletonAsset* skeletonAsset; //0x0008
	__int32 m_BoneCount; //0x0010

	char* GetBoneNameAt(int i)
	{
		if (i <= m_BoneCount)
			return skeletonAsset->BoneNames[i];
		return NULL;
	}
}; //Size: 0x0048

class ClientBoneCollisionComponent
{
public:
	UpdatePoseResultData m_ragdollTransforms; //0x0000
	char pad_0008[64]; //0x0008
	AnimationSkeleton* animationSkeleton; //0x0048

	D3DXVECTOR3 GetBonePos(int BoneId)
	{
		if (!IsValidPtr(this)) return D3DXVECTOR3(0, 0, 0);
		if (!this->m_ragdollTransforms.m_ValidTransforms) return D3DXVECTOR3(0, 0, 0);
		UpdatePoseResultData::QuatTransform* pQuat = this->m_ragdollTransforms.m_ActiveWorldTransforms;
		if (!IsValidPtr(pQuat)) return D3DXVECTOR3(0, 0, 0);
		return D3DXVECTOR3(pQuat[BoneId].m_TransAndScale.x, pQuat[BoneId].m_TransAndScale.y, pQuat[BoneId].m_TransAndScale.z);
	}

	/*bool GetBone(const int BoneId, D3DXVECTOR3& BoneOut)
	{
		UpdatePoseResultData PoseResult = this->m_ragdollTransforms;
		if (PoseResult.m_ValidTransforms)
		{
			UpdatePoseResultData::QuatTransform* pQuat = PoseResult.m_ActiveWorldTransforms;
			if (!IsValidPtr(pQuat))
				return false;

			D3DXVECTOR4 Bone = pQuat[BoneId].m_TransAndScale;
			BoneOut = D3DXVECTOR3(Bone.x, Bone.y, Bone.z);
			return true;
		}
		return false;
	}*/

	bool GetBone(const char* BoneName, D3DXVECTOR3& BoneOut)
	{
		int BoneId = -1;

		AnimationSkeleton* pSkeleton = animationSkeleton;
		if (!IsValidPtr(pSkeleton))
			return false;

		for (int i = 0; i < pSkeleton->m_BoneCount; i++)
		{
			char* name = pSkeleton->GetBoneNameAt(i);
			if (_stricmp(name, BoneName) == 0)
				BoneId = i;
		}

		if (BoneId == -1)
			return false;

		UpdatePoseResultData PoseResult = this->m_ragdollTransforms;
		if (PoseResult.m_ValidTransforms)
		{
			UpdatePoseResultData::QuatTransform* pQuat = PoseResult.m_ActiveWorldTransforms;
			if (!IsValidPtr(pQuat))
				return false;

			D3DXVECTOR4 Bone = pQuat[BoneId].m_TransAndScale;
			BoneOut = D3DXVECTOR3(Bone.x, Bone.y, Bone.z);
			return true;
		}
		return false;
	}
};

class WSClientSoldierEntity
{
public:
	ClientBoneCollisionComponent* getBoneCollision() {
		return GetClassFromName<ClientBoneCollisionComponent*>(this, "ClientBoneCollisionComponent");
	}
};

class ClientSoldier
{
public:
	char pad_0000[712]; //0x0000
	ClientSoldierHealthComponent* m_pClientSoldierHealthComponent; //0x02C8
	char pad_02D0[96]; //0x02D0
	WSClientSoldierEntity* WSClientSoldierEntity; //0x0330
	char pad_0338[412]; //0x0338
	float HeightOffset; //0x04D4
	char pad_04D8[640]; //0x04D8
	ClientSoldierPrediction* m_pClientSoldierPredicton; //0x0758
	char pad_0760[704]; //0x0760
	ClientAimEntity* m_pClientAimEntity; //0x0A20
	ClientSoldierWeapon* ClientSoldierWeapon; //0x0A28
	char pad_0A30[40]; //0x0A30
	__int8 Occluded; //0x0A58
	char pad_0A59[507]; //0x0A59
}; //Size: 0x0C54

class nameClass
{
public:
	char Name[20]; //0x0000
	char pad_0014[64]; //0x0014
}; //Size: 0x0054

class ClientPlayer
{
public:
	char pad_0000[24]; //0x0000
	nameClass* NameClass; //0x0018
	char pad_0020[56]; //0x0020
	uint32_t Team; //0x0058
	char pad_005C[420]; //0x005C
	DWORD64 AttachedControllable; //0x0200
	char pad_0208[8]; //0x0208
	ClientSoldier* ControlledControllable; //0x0210
//	char pad_0218[3624]; //0x0218
}; //Size: 0x1040

class mppPlayer
{
public:
	ClientPlayer* PlayerArray[60]; //0x0000
};

class PlayerManager
{
public:
	char pad_0000[1384]; //0x0000
	ClientPlayer* LocalPlayer; //0x0568
	char pad_0570[504]; //0x0570
	mppPlayer* mppPlayer; //0x0768
};

class GameContext {
public:
	char pad_0000[56]; //0x0000
	Level* level; //0x0038
	char pad_0040[24]; //0x0040
	PlayerManager* pPlayerManager; //0x0058
	static GameContext* Instance() {
		return *reinterpret_cast<GameContext**>(OFFSET_CLIENTGAMECONTEXT);
	}
};

class RenderView
{
public:
	char pad_0000[800]; //0x0000
	D3DXVECTOR3 CameraPos; //0x0320
	char pad_032C[260]; //0x032C
	D3DXMATRIXA16 viewProj; //0x0430
	//char pad_0470[288]; //0x0470
}; //Size: 0x0590

class GameRenderer {
public:
	char pad_0000[1336]; //0x0000
	RenderView* renderView; //0x538
	static GameRenderer* Instance() {
		return *reinterpret_cast<GameRenderer**>(OFFSET_GAMERENDERER);
	}
};

class UnknownPtr2
{
public:
	char pad_0000[168]; //0x0000
	float yaw; //0x00A8
	float pitch; //0x00AC
	char pad_00B0[12]; //0x00B0
}; //Size: 0x00BC

class UnknownPtr1
{
public:
	char pad_0000[56]; //0x0000
	class UnknownPtr2* UnknownPtr2; //0x0038
	char pad_0040[8]; //0x0040
}; //Size: 0x0048

class LocalAimer
{
public:
	char pad_0000[152]; //0x0000
	class UnknownPtr1* UnknownPtr1; //0x0098
	char pad_00A0[8]; //0x00A0

	static LocalAimer* Instance() {
		return *reinterpret_cast<LocalAimer**>(OFFSET_AIMER);
	}
}; //Size: 0x00A8

class HelpfulShit
{
public:
	class GameTimeSettings* m_GameTimeSettings; //0x0000
	class GameRenderSettings* m_GameRenderSettings; //0x0008
	class UISettings* m_UISettings; //0x0010
	class GameSettings* m_GameSettings; //0x0018
	class NetworkSettings* m_NetworkSettings; //0x0020
	char pad_0028[56]; //0x0028
	class ServerSettings* m_ServerSettings; //0x0060
	class ClientSettings* m_ClientSettings; //0x0068
	char pad_0070[24]; //0x0070
	class WindowSettings* m_WindowSettings; //0x0088
	class DX11DisplaySettings* m_DX11DisplaySettings; //0x0090
	class DiceJobSchedulerSettings* m_DiceJobSchedulerSettings; //0x0098
	class OnlineSettings* m_OnlineSettings; //0x00A0
	char pad_00A8[8]; //0x00A8
	class EntitySettings* m_EntitySettings; //0x00B0
	class WSLiveContentUpdateSettings* m_WSLiveContentUpdateSettings; //0x00B8
	char pad_00C0[896]; //0x00C0
	
	static HelpfulShit* Instance() {
		return *reinterpret_cast<HelpfulShit**>(OFFSET_HELPFULSHIT);
	}
}; //Size: 0x0440

//struct ViewAngleStruct
//{
//	char pad[0x68];
//	float yaw;
//	float pitch;
//};