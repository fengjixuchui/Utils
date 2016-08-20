// vc4hk.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "../vc3hk/VMTHook.h"
#include "x64_sdk.h"
#include "DXTK/Inc/SimpleMath.h"
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
DWORD WINAPI BF3HookThread(LPVOID)
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
		CreateThread(NULL, NULL, BF3HookThread, NULL, NULL, NULL);
#endif // DEBUG

		//CloseHandle(CreateThread(NULL, 0, &HookThread, (void*)1, 0, NULL)); //Enable hook
		PresentHook = new CVMTHookManager((DWORD**)fb::DxRenderer::GetInstance()->m_pScreen->m_pSwapChain);
		oPresent = (tPresent)PresentHook->dwGetMethodAddress(8);
		PresentHook->dwHookMethod((DWORD)hkPresent, 8); break; }
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:		
	case DLL_PROCESS_DETACH:if (PresentHook != nullptr)PresentHook->UnHook(); break;
		break;
	}
	return TRUE;
}
