// vc4hk.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "../vc3hk/VMTHook.h"
#include "x64_sdk.h"

#pragma region


CVMTHookManager* PresentHook = 0;
typedef HRESULT(__stdcall* tPresent)(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags);
tPresent oPresent; //our dx hook
#pragma endregion Hook

int Count = 0;
bool Initialized = false;
bool DrawMenu = false;

void _stdcall SoldierWeaponUpgrade() {
	fb::ClientPlayer* pLocalPlayer = GetLocalPlayer();
	if (!POINTERCHK(pLocalPlayer))
		return;

	if (pLocalPlayer->InVehicle())
		return;

	fb::ClientSoldierEntity* pLocalSoldier = pLocalPlayer->GetSoldierEntity();
	if (!POINTERCHK(pLocalSoldier))
		return;
	fb::BreathControlHandler *pBreath= pLocalSoldier->m_breathControlHandler ;
	if (POINTERCHK(pBreath)) {
		pBreath->m_breathControlTimer = 0.001f;
			pBreath->m_Enabled = false;
	}
	

	fb::SoldierWeaponComponent* pWepComp = pLocalSoldier->m_pWeaponComponent;
	if (!POINTERCHK(pWepComp))
		return;

	fb::SoldierWeapon* pWeapon = pWepComp->GetActiveSoldierWeapon();
	if (!POINTERCHK(pWeapon))
		return;

	fb::WeaponFiring* pFiring = pWeapon->m_pPrimary;
	if (!POINTERCHK(pFiring))
		return;

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
	case DLL_PROCESS_ATTACH: 

		DisableThreadLibraryCalls(hModule);
		//CreateThread(NULL, NULL, PBSSThread, NULL, NULL, NULL);

#ifdef _DEBUG
		CreateThread(NULL, NULL, BF4HookThread, NULL, NULL, NULL);
#endif // DEBUG

		//CloseHandle(CreateThread(NULL, 0, &HookThread, (void*)1, 0, NULL)); //Enable hook
		PresentHook = new CVMTHookManager((intptr_t**)fb::DxRenderer::GetInstance()->m_pScreen->m_pSwapChain);
		oPresent = (tPresent)PresentHook->dwGetMethodAddress(8);
		PresentHook->dwHookMethod((intptr_t)hkPresent, 8); break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:		
	case DLL_PROCESS_DETACH:if (PresentHook != nullptr)PresentHook->UnHook();
		break;
	}
	return TRUE;
}
