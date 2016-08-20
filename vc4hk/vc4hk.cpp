// vc4hk.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "..\vc3hk\VMTHook.h"
#include "x64_sdk.h"
#pragma region
CVMTHookManager* PreFrameHook = 0;
typedef int(__stdcall* tPreFrameUpdate)(float dt);
tPreFrameUpdate oPreFrameUpdate = 0;

CVMTHookManager* PresentHook = 0;
typedef signed int(__stdcall* tPresent)(int, int, int);
tPresent oPresent;
#pragma endregion Hook

signed int __stdcall hkPresent(int a1, int a2, int a3) {


	//ButtonMenu();


	SoldierWeaponUpgrade();
	VehicleWeaponUpgrade();

	PlayerIteration();
	EntityWorld();





	return oPresent(a1, a2, a3);
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
		CreateThread(NULL, NULL, BF3HookThread, NULL, NULL, NULL);
#endif // DEBUG

		//CloseHandle(CreateThread(NULL, 0, &HookThread, (void*)1, 0, NULL)); //Enable hook
		PresentHook = new CVMTHookManager((DWORD**)fb::DxRenderer::GetInstance()->m_pScreen->m_pSwapChain);
		oPresent = (tPresent)PresentHook->dwGetMethodAddress(8);
		PresentHook->dwHookMethod((DWORD)hkPresent, 8); break; }
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:		if (PresentHook != nullptr)PresentHook->UnHook(); break;
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
