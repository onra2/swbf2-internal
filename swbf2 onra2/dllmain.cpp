#include <cstdio>
#include "SDK.h"
#include "D2DOverlay.h"
#include "Menu.h"
#include "MinHook/include/MinHook.h"
#include <chrono>
#pragma comment(lib, "libMinHook-x64-v141-mtd.lib")

void Drawer(Direct2DOverlay* ov);
HINSTANCE asdd;
Menu menu;
HMODULE DllInst = nullptr;
using namespace std;

typedef BOOL(WINAPI* BITBLT)(HDC hdc, int x, int y, int cx, int cy, HDC hdcSrc, int x1, int y1, DWORD rop);
BITBLT oBitBlt = nullptr;
//block ff screenshots
bool hkBitBlt(const HDC hdc, const int x, const int y, const int cx, const int cy, const HDC hdcSrc, const int x1, const int y1, const DWORD rop)
{
	HWND targetWindow = FindWindow("lol", "kek");
	ShowWindow(targetWindow, SW_HIDE);
	auto result = oBitBlt(hdc, x, y, cx, cy, hdcSrc, x1, y1, rop);
	ShowWindow(targetWindow, SW_SHOW);
	auto now = std::chrono::system_clock::now();
	std::time_t Today = std::chrono::system_clock::to_time_t(now);
	std::cout << "\n\tScreenshot Taken by FairFight at " << std::ctime(&Today) << "\n";
	return result;
}

#pragma region Settings
RenderView* pRenderer;

D3DXCOLOR Friend = D3DXCOLOR(0, 0, 1, 0);
D3DXCOLOR Enemy = D3DXCOLOR(1, 0, 0, 0);
D3DXCOLOR Visible = D3DXCOLOR(0, 1, 0, 0);

bool PlayerVisible = false;

int vsync = true;
int ticks = 0;
int beforeclock = 0;

int enemyboxes = true;
int teamboxes = false;
int enemysnaplines = false;
int enemydistance = false;
int teamdistance = false;
int teamsnaplines = false;
int enemyvehicles = false;
int teamvehicles = false;
int enemyhealth = false;
int teamhealth = false;
int enemynames = true;
int teamnames = false;
int aimbot = false;
bool headshot = true;
#pragma endregion

void Drawer(Direct2DOverlay* ov)
{
	if (GetAsyncKeyState(VK_END)) {
		FreeConsole();
		MH_Uninitialize();
		//RegHook::DestroyAllHooks();
		FreeLibraryAndExitThread(DllInst, 0);
	}
	
	int vsyncA = clock();
	ticks += 1;
	if (beforeclock == 0) {
		beforeclock = clock();
	}
	ov->BeginDraw();
	ov->ClearScreen();

	menu.Render();

	RECT Overlay = ov->GetOverlayRect();

	D3DXVECTOR3 ScreenMiddle = { float(Overlay.right - Overlay.left) / 2 , float(Overlay.bottom - Overlay.top) / 2 , 0 };

	GameRenderer* pGameRenderer = GameRenderer::Instance();
	if (!IsValidPtr(pGameRenderer) || !IsValidPtr(pGameRenderer->renderView)) return;

	pRenderer = pGameRenderer->renderView;

	PlayerManager* pPlayerManager = GameContext::Instance()->pPlayerManager;
	ClientPlayer* pLocalPlayer = pPlayerManager->LocalPlayer;
	if (!IsValidPtr(pLocalPlayer)) return;

	nameClass* p_name_class = pLocalPlayer->NameClass;
	if (!IsValidPtr(p_name_class)) return;

	for (auto pPlayer : pPlayerManager->mppPlayer->PlayerArray)
	{
		if (!IsValidPtr(pPlayer)) continue;

		if (pPlayer == pLocalPlayer) continue;

		ClientSoldier* pLocalSoldier = pLocalPlayer->ControlledControllable;
		if (!IsValidPtr(pLocalSoldier)) continue;
		ClientSoldier* pSoldier = pPlayer->ControlledControllable;
		if (!IsValidPtr(pSoldier)) continue;

		if (!IsValidPtr(pSoldier->m_pClientSoldierHealthComponent)) continue;
		if (pSoldier->m_pClientSoldierHealthComponent->Health <= 0) continue;

		PlayerVisible = 1 - pSoldier->Occluded;

		ClientSoldierPrediction* LocalSoldierPrediction = pLocalSoldier->m_pClientSoldierPredicton;
		if (!IsValidPtr(LocalSoldierPrediction)) continue;

		ClientSoldierPrediction* PlayerSoldierPrediction = pSoldier->m_pClientSoldierPredicton;
		if (!IsValidPtr(PlayerSoldierPrediction)) continue;

		D3DXCOLOR Chosencolor = D3DXCOLOR();
		if (pPlayer->Team == pLocalPlayer->Team) { Chosencolor = Friend; }
		else { PlayerVisible ? Chosencolor = Visible : Chosencolor = Enemy; }

		D3DXVECTOR3 head = PlayerSoldierPrediction->Location;
		head.y += pSoldier->HeightOffset + .25;
		D3DXVECTOR3 foot = PlayerSoldierPrediction->Location;

		if (WorldToScreen(Overlay, foot, pRenderer) && WorldToScreen(Overlay, head, pRenderer))
		{
			float heightoffset = Distance3D(foot, head);
			if (pPlayer->Team == pLocalPlayer->Team && teamboxes || pPlayer->Team != pLocalPlayer->Team && enemyboxes)
			{
				float factor = (heightoffset / 5);

				D3DXVECTOR3 m2 = D3DXVECTOR3(head.x - factor, head.y, 0);
				D3DXVECTOR3 m1 = D3DXVECTOR3(head.x + factor, head.y, 0);
				D3DXVECTOR3 m3 = D3DXVECTOR3(foot.x - factor, foot.y, 0);
				D3DXVECTOR3 m4 = D3DXVECTOR3(foot.x + factor, foot.y, 0);

				int thicc = 1;

				ov->DrawLine(m1.x, m1.y, m2.x, m2.y, thicc, Chosencolor.r, Chosencolor.g, Chosencolor.b);
				ov->DrawLine(m2.x, m2.y, m3.x, m3.y, thicc, Chosencolor.r, Chosencolor.g, Chosencolor.b);
				ov->DrawLine(m3.x, m3.y, m4.x, m4.y, thicc, Chosencolor.r, Chosencolor.g, Chosencolor.b);
				ov->DrawLine(m4.x, m4.y, m1.x, m1.y, thicc, Chosencolor.r, Chosencolor.g, Chosencolor.b);
			}
			if (pPlayer->Team == pLocalPlayer->Team && teamhealth || pPlayer->Team != pLocalPlayer->Team && enemyhealth)
			{
				float factor = (heightoffset / 3);
				D3DXVECTOR3 m1 = D3DXVECTOR3(head.x + factor, head.y, 0);
				D3DXVECTOR3 m2 = D3DXVECTOR3(foot.x + factor, foot.y, 0);

				float perc = (pSoldier->m_pClientSoldierHealthComponent->Health / pSoldier->m_pClientSoldierHealthComponent->m_flMaxHealth);

				float thicc = heightoffset / 8;
				if (thicc < 4) thicc = 4;
				D3DXCOLOR HealthColor = D3DXCOLOR(1 - perc, perc, 0, 1);

				ov->DrawLine(m1.x + thicc / 2, m1.y - 2, m2.x + thicc / 2, m2.y + 2, thicc + 2, 0, 0, 0);
				ov->DrawLine(m1.x + thicc / 2, m1.y + ((m2.y - m1.y) * (1 - perc)), m2.x + thicc / 2, m2.y, thicc, HealthColor.r, HealthColor.g, HealthColor.b);
			}
			std::wstring name;
			if (pPlayer->Team == pLocalPlayer->Team && teamnames || pPlayer->Team != pLocalPlayer->Team && enemynames) {
				int fontsize = 15;			
				try {
					std::wstring tmpname = std::wstring(&pPlayer->NameClass->Name[0], &pPlayer->NameClass->Name[20]);
					auto czech = wcstok(&tmpname[0], L" ");
					if (czech != nullptr) { name = czech; }	//nice meme
				}
				catch (const std::exception & exc) {}
				float offset = name.length() * fontsize / 5;
				ov->DrawString(name, fontsize, head.x - offset, head.y - (heightoffset / 4) - fontsize, Chosencolor.r, Chosencolor.g, Chosencolor.b);
			}
			if (pPlayer->Team == pLocalPlayer->Team && teamsnaplines || pPlayer->Team != pLocalPlayer->Team && enemysnaplines) {
				ov->DrawLine(headshot == true ? head.x : foot.x, headshot == true ? head.y : foot.y,
					ScreenMiddle.x, ScreenMiddle.y, 1, Chosencolor.r, Chosencolor.g, Chosencolor.b);
			}
			if (pPlayer->Team == pLocalPlayer->Team && teamdistance || pPlayer->Team != pLocalPlayer->Team && enemydistance)
			{
				int fontsize = 10;
				float factor = heightoffset / 3;
				float distance = Distance3D(LocalSoldierPrediction->Location, PlayerSoldierPrediction->Location);
				ov->DrawString(L"Distance: " + std::to_wstring(int(distance)) + L"m"
					, fontsize, foot.x - factor, foot.y
					, Chosencolor.r, Chosencolor.g, Chosencolor.b);
			}

			/*auto LocalBoneCollision = pLocalSoldier->getBoneCollision();
			std::cout << std::hex << LocalBoneCollision << std::endl;*/

			/*if (!name.empty())
			{
				auto pWSClientSoldierEntity = pSoldier->WSClientSoldierEntity;
				if (!IsValidPtr(pWSClientSoldierEntity)) continue;
				auto BoneCollision = pWSClientSoldierEntity->getBoneCollision();
				D3DXVECTOR3 bone = BoneCollision->GetBonePos(Head);
				D3DXVECTOR2 angle = CalcAim(bone, pRenderer);
				std::cout << std::hex << BoneCollision;
				std::wcout << L": " + name + L"| X: " << angle.x << L" Y: " << angle.y << std::endl;
			}*/
			
			ClientSoldierWeapon* pWeapon = pLocalPlayer->ControlledControllable->ClientSoldierWeapon;
			if (!IsValidPtr(pWeapon)) continue;
			SoldierWeaponData* pWeaponData = pWeapon->soldierWeaponData;
			if (!IsValidPtr(pWeaponData)) continue;
			WeaponFiring* pWeaponFiring = pWeaponData->weaponFiring;
			if (!IsValidPtr(pWeaponFiring)) continue;
			FiringFunctionData* pFiringFunctionData = pWeaponFiring->firingFunctionData;
			if (!IsValidPtr(pFiringFunctionData)) continue;

			pFiringFunctionData->BulletSpeed = 750;
		}

		if (pPlayer->Team != pLocalPlayer->Team && aimbot && (GetAsyncKeyState(VK_RBUTTON) || GetAsyncKeyState(VK_LBUTTON)))
		{
			if (!PlayerVisible) continue;
			if (PlayerSoldierPrediction->Location.x == NULL) continue;
			if (pSoldier == nullptr) continue;
			if (pSoldier->m_pClientSoldierHealthComponent->Health == NULL) continue;
			if (pPlayer->ControlledControllable == 0) continue;
			if (pPlayer->AttachedControllable != 0) continue;

			ClientSoldierWeapon* pWeapon = pLocalPlayer->ControlledControllable->ClientSoldierWeapon;
			if (!IsValidPtr(pWeapon)) continue;
			SoldierWeaponData* pWeaponData = pWeapon->soldierWeaponData;
			if (!IsValidPtr(pWeaponData)) continue;
			WeaponFiring* pWeaponFiring = pWeaponData->weaponFiring;
			if (!IsValidPtr(pWeaponFiring)) continue;
			FiringFunctionData* pFiringFunctionData = pWeaponFiring->firingFunctionData;
			if (!IsValidPtr(pFiringFunctionData)) continue;

			auto speed = pFiringFunctionData->BulletSpeed;
			float distance = Distance3D(LocalSoldierPrediction->Location, PlayerSoldierPrediction->Location);
			float timeToImpact = distance / speed;
			
			D3DXVECTOR3 Aimhead = PlayerSoldierPrediction->Location;
			Aimhead.y += pSoldier->HeightOffset - .15;
			/*D3DXVECTOR3 AimBody = PlayerSoldierPrediction->Location;
			AimBody.y += pSoldier->HeightOffset * .7;*/

			Aimhead += PlayerSoldierPrediction->Velocity * timeToImpact;
			//AimBody += PlayerSoldierPrediction->Velocity * timeToImpact;

			D3DXVECTOR2 angle = CalcAim(Aimhead, pRenderer);
			auto localAim = LocalAimer::Instance()->UnknownPtr1->UnknownPtr2;
			if (localAim->yaw > 0 && localAim->yaw < 6.283f)
			{
				localAim->yaw = angle.y;
				localAim->pitch = angle.x;
			}
		}
		
		//auto BosskSkin = GameContext::Instance()->level->Teams->TeamDark->TeamData->Heroes->HeroArray[0]->Skin;
		//auto BosskChoose = GameContext::Instance()->level->Teams->TeamDark->TeamData->Heroes->HeroArray[0]->AnimationFitCharacterChoose;
		//auto BosskGameplay = GameContext::Instance()->level->Teams->TeamDark->TeamData->Heroes->HeroArray[0]->AnimationFitGameplay;
		//GameContext::Instance()->level->Teams->TeamLight->TeamData->Soldiers->SoldierClasses->Assault->AnimationFitCharacterChoose = BosskChoose;
		//GameContext::Instance()->level->Teams->TeamLight->TeamData->Soldiers->SoldierClasses->Assault->AnimationFitGameplay = BosskGameplay;
		//GameContext::Instance()->level->Teams->TeamLight->TeamData->Soldiers->SoldierClasses->Assault->Skin = BosskSkin;
		//GameContext::Instance()->level->Teams->TeamLight->TeamData->Soldiers->SoldierClasses->Heavy->AnimationFitCharacterChoose = BosskChoose;
		//GameContext::Instance()->level->Teams->TeamLight->TeamData->Soldiers->SoldierClasses->Heavy->AnimationFitGameplay = BosskGameplay;
		//GameContext::Instance()->level->Teams->TeamLight->TeamData->Soldiers->SoldierClasses->Heavy->Skin = BosskSkin;
		//GameContext::Instance()->level->Teams->TeamLight->TeamData->Soldiers->SoldierClasses->Officer->AnimationFitCharacterChoose = BosskChoose;
		//GameContext::Instance()->level->Teams->TeamLight->TeamData->Soldiers->SoldierClasses->Officer->AnimationFitGameplay = BosskGameplay;
		//GameContext::Instance()->level->Teams->TeamLight->TeamData->Soldiers->SoldierClasses->Officer->Skin = BosskSkin;
		//GameContext::Instance()->level->Teams->TeamLight->TeamData->Soldiers->SoldierClasses->Specialist->AnimationFitCharacterChoose = BosskChoose;
		//GameContext::Instance()->level->Teams->TeamLight->TeamData->Soldiers->SoldierClasses->Specialist->AnimationFitGameplay = BosskGameplay;
		//GameContext::Instance()->level->Teams->TeamLight->TeamData->Soldiers->SoldierClasses->Specialist->Skin = BosskSkin;
		////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
		//GameContext::Instance()->level->Teams->TeamDark->TeamData->Soldiers->SoldierClasses->Assault->AnimationFitCharacterChoose = BosskChoose;
		//GameContext::Instance()->level->Teams->TeamDark->TeamData->Soldiers->SoldierClasses->Assault->AnimationFitGameplay = BosskGameplay;
		//GameContext::Instance()->level->Teams->TeamDark->TeamData->Soldiers->SoldierClasses->Assault->Skin = BosskSkin;
		//GameContext::Instance()->level->Teams->TeamDark->TeamData->Soldiers->SoldierClasses->Heavy->AnimationFitCharacterChoose = BosskChoose;
		//GameContext::Instance()->level->Teams->TeamDark->TeamData->Soldiers->SoldierClasses->Heavy->AnimationFitGameplay = BosskGameplay;
		//GameContext::Instance()->level->Teams->TeamDark->TeamData->Soldiers->SoldierClasses->Heavy->Skin = BosskSkin;
		//GameContext::Instance()->level->Teams->TeamDark->TeamData->Soldiers->SoldierClasses->Officer->AnimationFitCharacterChoose = BosskChoose;
		//GameContext::Instance()->level->Teams->TeamDark->TeamData->Soldiers->SoldierClasses->Officer->AnimationFitGameplay = BosskGameplay;
		//GameContext::Instance()->level->Teams->TeamDark->TeamData->Soldiers->SoldierClasses->Officer->Skin = BosskSkin;
		//GameContext::Instance()->level->Teams->TeamDark->TeamData->Soldiers->SoldierClasses->Specialist->AnimationFitCharacterChoose = BosskChoose;
		//GameContext::Instance()->level->Teams->TeamDark->TeamData->Soldiers->SoldierClasses->Specialist->AnimationFitGameplay = BosskGameplay;
		//GameContext::Instance()->level->Teams->TeamDark->TeamData->Soldiers->SoldierClasses->Specialist->Skin = BosskSkin;
	}

	ov->EndDraw();
	if (vsync) {
		int pausetime = 15 - (clock() - vsyncA);
		if (pausetime > 0 && pausetime < 30) {
			Sleep(pausetime);
		}
	}
}

DWORD WINAPI OverlayStart(LPVOID lpParam)
{
	AllocConsole();
	FILE* file = nullptr;
	freopen_s(&file, "CONIN$", "r", stdin);
	freopen_s(&file, "CONOUT$", "w", stdout);
	//DumpBoneIDs();
	//std::cout << "Fairfight logs:";

	MH_Initialize();
	MH_CreateHook(&BitBlt, &hkBitBlt, reinterpret_cast<LPVOID*>(&oBitBlt));
	MH_EnableHook(&BitBlt);

	writeNops(OFFSET_SPREADPATCH, 5);
	bytePatch(OFFSET_RECOILPATCH - 6, { 0xe9, 0xe8, 0x00, 0x00, 0x00, 0x90 });

	//thx coltonon :pog:
	auto* asd = new Direct2DOverlay(asdd, Drawer);
	asd->Initialize("STAR WARS Battlefront II", "kek");

	menu.Initialize(asd, L"kek");

	MenuTab esptab(L"Enemy ESP");
	esptab.AddItem(MenuItem(L"Enemy Boxes", &enemyboxes));
	esptab.AddItem(MenuItem(L"Enemy SnapLines", &enemysnaplines));
	esptab.AddItem(MenuItem(L"Enemy Healthbars", &enemyhealth));
	esptab.AddItem(MenuItem(L"Enemy Names", &enemynames));
	esptab.AddItem(MenuItem(L"Enemy Distance", &enemydistance));

	MenuTab teamtab(L"Team ESP");
	teamtab.AddItem(MenuItem(L"Team Boxes", &teamboxes));
	teamtab.AddItem(MenuItem(L"Team SnapLines", &teamsnaplines));
	teamtab.AddItem(MenuItem(L"Team Healthbars", &teamhealth));
	teamtab.AddItem(MenuItem(L"Team Names", &teamnames));
	teamtab.AddItem(MenuItem(L"Team Distance", &teamdistance));

	MenuTab aimtab(L"Aimbot");
	aimtab.AddItem(MenuItem(L"Aimbot", &aimbot));

	MenuTab misctab(L"Misc");
	misctab.AddItem(MenuItem(L"vSync", &vsync));
	menu.AddTab(esptab);
	menu.AddTab(teamtab);
	menu.AddTab(aimtab);
	menu.AddTab(misctab);
	
	asd->StartLoop();
	return 0;
}

BOOL APIENTRY DllMain(const HMODULE hModule, const DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	UNREFERENCED_PARAMETER(lpReserved);

	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
		//DisableThreadLibraryCalls(hModule);
		DllInst = hModule;
		CreateThread(nullptr, NULL, OverlayStart, hModule, NULL, nullptr);
		//CloseHandle(OverlayStart);
		//CloseHandle(hModule);
		break;
	case DLL_THREAD_ATTACH:
		break;
	case DLL_THREAD_DETACH:
		break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}