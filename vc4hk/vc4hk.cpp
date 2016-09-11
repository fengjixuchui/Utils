// vc4hk.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "../vc3hk/VMTHook.h"
#include "x64_sdk.h"

#pragma region


CVMTHookManager* PresentHook = 0;
typedef HRESULT(__stdcall* tPresent)(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags);
tPresent oPresent; 


CVMTHookManager* PreFrameHook = 0;
typedef int(__stdcall* tPreFrameUpdate)(float dt);
tPreFrameUpdate oPreFrameUpdate = 0;

fb::BorderInputNode* g_pBorderInputNode = fb::BorderInputNode::GetInstance();

#pragma endregion Hook

int Count = 0;
bool Initialized = false;
bool DrawMenu = false;
//void _stdcall PlayerIteration()
//{
//	
//		fb::ClientGameContext* g_pGameContext = (fb::ClientGameContext*)OFFSET_CLIENTGAMECONTEXT;
//		if (!POINTERCHK(g_pGameContext)) return;
//
//		fb::ClientPlayerManager* pPlayerManager = g_pGameContext->m_pPlayerManager;
//		if (!POINTERCHK(pPlayerManager) || pPlayerManager->m_players.empty()) return;
//
//		eastl::vector<fb::ClientPlayer*> pVecCP = pPlayerManager->m_players;
//		if (pVecCP.empty()) return;
//
//		//fb::ClientSoldierEntity* pMySoldier = pPlayerManager->m_localPlayer->getSoldierEnt();
//
//		//if (!POINTERCHK(pMySoldier))
//		//	return;
//		//	if (!isalive(pMySoldier->Alive())) return;
//		int size = pVecCP.size();
//		for (int i = 0; i < size; i++)
//		{
//			fb::ClientPlayer* pClientPlayer = pVecCP.at(i);
//			fb::ClientSoldierEntity* pSoldier = pClientPlayer->getSoldierEnt();
//			if (!POINTERCHK(pSoldier)) continue;
//
//			if (pSoldier->isInVehicle()) continue;
//
//			
//
//			
//				fb::ClientSpottingTargetComponent* pCSTC = pSoldier->getComponent<fb::ClientSpottingTargetComponent>("ClientSpottingTargetComponent");
//				if (POINTERCHK(pCSTC)) {
//					if (pCSTC->m_spotType == fb::SpotType_None) { pCSTC->m_spotType = fb::SpotType_Passive; }
//					else
//					{
//						continue;
//					}
//				}
//			
//		}
//	}

void _stdcall SoldierWeaponUpgrade() {


    //unlock
	LPVOID unlock = *(LPVOID*)OFFSET_SYNCEDBFSETTINGS;
	if (POINTERCHK(unlock)) {
	
		*(bool* )((intptr_t)unlock + 0x54) = true;
	}
		
	fb::ClientPlayer* pLocalPlayer = GetLocalPlayer();
	if (!POINTERCHK(pLocalPlayer))
		return;

	if (pLocalPlayer->InVehicle())
		return;

	fb::ClientSoldierEntity* pLocalSoldier = pLocalPlayer->GetSoldierEntity();
	if (!POINTERCHK(pLocalSoldier))
		return;

	fb::BreathControlHandler *pBreath = (fb::BreathControlHandler *)((intptr_t)pLocalSoldier+0x588);
	if (POINTERCHK(pBreath)) {
		pBreath->m_breathControlTimer = 0.00125f;
		pBreath->m_Enabled = false;
	}
	

	fb::SoldierWeaponComponent* pWepComp = pLocalSoldier->m_pWeaponComponent;
	if (!POINTERCHK(pWepComp))
		return;

	fb::SoldierWeapon* pLW = pWepComp->GetActiveSoldierWeapon();
	if (!POINTERCHK(pLW))
		return;

	fb::WeaponFiring* pFiring = pLW->m_pPrimary;
	if (!POINTERCHK(pFiring))
		return;
	if (!POINTERCHK(pLW->m_authorativeAiming))
		return;

	if (pLW->m_authorativeAiming->m_zoomLevel > 0)
	{
		if (!POINTERCHK(pLW->m_authorativeAiming->m_data))return;
		

			if (POINTERCHK(pLW->m_authorativeAiming->m_data->m_ZoomLevels[0]))
			{
				fb::ZoomLevelData *zoomLevel = pLW->m_authorativeAiming->m_data->m_ZoomLevels[0];
				if (POINTERCHK(zoomLevel))
				{

					zoomLevel->m_SuppressedSwayPitchMagnitude = 0.0f;
					zoomLevel->m_SuppressedSwayMinFactor = 0.0f;
					zoomLevel->m_SuppressedSwayPitchMagnitude = 0.0f;
					zoomLevel->m_SuppressedSwayYawMagnitude = 0.0f;
					zoomLevel->m_SwayPitchMagnitude = 0.0f;
					zoomLevel->m_SwayYawMagnitude = 0.0f;
					//zoomLevel->m_supportedSwayPitchMlt = 0.0f;
					//zoomLevel->m_supportedSwayYawMlt = 0.0f;
				}
			}
			if (POINTERCHK(pLW->m_authorativeAiming->m_data->m_ZoomLevels[1]))
			{
				fb::ZoomLevelData *zoomLevel1 = pLW->m_authorativeAiming->m_data->m_ZoomLevels[1];
				if (POINTERCHK(zoomLevel1))
				{
					zoomLevel1->m_SuppressedSwayPitchMagnitude = 0.0f;
					zoomLevel1->m_SuppressedSwayMinFactor = 0.0f;
					zoomLevel1->m_SuppressedSwayPitchMagnitude = 0.0f;
					zoomLevel1->m_SuppressedSwayYawMagnitude = 0.0f;
					zoomLevel1->m_SwayPitchMagnitude = 0.0f;
					zoomLevel1->m_SwayYawMagnitude = 0.0f;
					//zoomLevel1->m_supportedSwayPitchMlt = 0.0f;
					//zoomLevel1->m_supportedSwayYawMlt = 0.0f;
				}
			
		}
	}
	fb::WeaponSway* pSway = pFiring->m_Sway;
	if (!POINTERCHK(pSway))
		return;

	fb::GunSwayData* pSwayData = pSway->m_Data;
	if (!POINTERCHK(pSwayData))
		return;
//	memset((void*)0x1409A5354, 0x90, 4);
//	memset((void*)0x1409A535C, 0x90, 2);
	pSwayData->m_DeviationScaleFactorZoom = 0.f;
	pSwayData->m_GameplayDeviationScaleFactorZoom = 0.f;
	pSwayData->m_DeviationScaleFactorNoZoom = 0.f;
	pSwayData->m_GameplayDeviationScaleFactorNoZoom = 0.f;
	pSwayData->m_FirstShotRecoilMultiplier = 0.f;
	pSwayData->m_ShootingRecoilDecreaseScale = 100;
}
int WINAPI hkPreFrame(float DeltaTime)
{
	
	int returnval = oPreFrameUpdate(DeltaTime);

	//if (GetAsyncKeyState(VK_RMENU) & 0x8000) {
	//	bAimHead = !bAimHead;

	//};
	//if (GetAsyncKeyState(VK_LMENU) & 0x8000) {
	//	AimKeyPressed = true;
	//}
	//else { AimKeyPressed = false; }

	//if (bAimbot &&  AimKeyPressed) {
	//	Aimbot();
	//}
	//else {
	//	LockOnEme = false;
	//	LockOn_pEnemySoldier = nullptr;
	//}

	//for (int i = 0; i < 123; i++)
	//{
	//	g_pBorderInputNode->m_inputCache->flInputBuffer[i] += g_pInputBuffers[i];
	//	g_pInputBuffers[i] = 0.0f;
	//}



	return returnval;
}
HRESULT __stdcall hkPresent(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags) {

	
	SoldierWeaponUpgrade();






	return oPresent(thisptr, SyncInterval, Flags);
}
void CreateConsole()
{
	int hConHandle = 0;
	HANDLE lStdHandle = 0;
	FILE *fp = 0;
	AllocConsole();
	// redirect unbuffered STDOUT to the console
	lStdHandle = GetStdHandle(STD_OUTPUT_HANDLE);
	hConHandle = _open_osfhandle(PtrToUlong(lStdHandle), _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	freopen_s(&fp, "CONOUT$", "w", stdout);

	// redirect unbuffered STDIN to the console
	lStdHandle = GetStdHandle(STD_INPUT_HANDLE);
	hConHandle = _open_osfhandle(PtrToUlong(lStdHandle), _O_TEXT);
	fp = _fdopen(hConHandle, "r");
	freopen_s(&fp, "CONOUT$", "r", stdin);

	// redirect unbuffered STDERR to the console
	lStdHandle = GetStdHandle(STD_ERROR_HANDLE);
	hConHandle = _open_osfhandle(PtrToUlong(lStdHandle), _O_TEXT);
	fp = _fdopen(hConHandle, "w");
	freopen_s(&fp, "CONOUT$", "w", stderr);
}
DWORD WINAPI BF4HookThread(LPVOID)
{

	CreateConsole();


	printf_s("[Battlefield 4]\n");
	return 0;
}
BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: {

		DisableThreadLibraryCalls(hModule);
		//CreateThread(NULL, NULL, PBSSThread, NULL, NULL, NULL);

#ifdef _DEBUG
		CreateThread(NULL, NULL, BF4HookThread, NULL, NULL, NULL);
#endif // DEBUG

#ifndef _DEBUG
		PreFrameHook = new CVMTHookManager((intptr_t**)fb::BorderInputNode::GetInstance());
		oPreFrameUpdate = (tPreFrameUpdate)PreFrameHook->dwGetMethodAddress(3);
		PreFrameHook->dwHookMethod((intptr_t)hkPreFrame, 3);


#endif // DEBUG	//CloseHandle(CreateThread(NULL, 0, &HookThread, (void*)1, 0, NULL)); //Enable hook
		PresentHook = new CVMTHookManager((intptr_t**)fb::DxRenderer::GetInstance()->m_pScreen->m_pSwapChain);
		oPresent = (tPresent)PresentHook->dwGetMethodAddress(8);
		PresentHook->dwHookMethod((intptr_t)hkPresent, 8);

		LPVOID IcmpCreateFile = (LPVOID)((intptr_t)GetProcAddress(GetModuleHandleW(L"iphlpapi.dll"), "IcmpCreateFile") + 0x327);

		DWORD dwOld;
		//ping spoof
		if (POINTERCHK(IcmpCreateFile)) {
			VirtualProtect((LPVOID)IcmpCreateFile, 3 * sizeof(BYTE), PAGE_EXECUTE_READWRITE, &dwOld);

			memset((LPVOID)IcmpCreateFile, 0x31, 1);
			memset((LPVOID)((intptr_t)IcmpCreateFile + 1), 0xc0, 1);//xor eax,eax
			memset((LPVOID)((intptr_t)IcmpCreateFile + 2), 0x90, 1);//inc eax

			VirtualProtect((LPVOID)IcmpCreateFile, 3 * sizeof(BYTE), dwOld, NULL);

			
		}break; }
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:		
	case DLL_PROCESS_DETACH:if (PresentHook != nullptr)PresentHook->UnHook();
		break;
	}
	return TRUE;
}
