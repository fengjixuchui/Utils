// vc4hk.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "../vc3hk/VMTHook.h"
#include "x64_sdk.h"
#include "Functions4.h"
#include<stdio.h>
#include <fstream>



#define LOGGG

#ifdef LOGGG
#define PRINT(code) code
#else
#define PRINT(code) 
#endif


#pragma region

CVMTHookManager* PresentHook = 0;
typedef HRESULT(__stdcall* tPresent)(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags);
tPresent oPresent;


CVMTHookManager* PreFrameHook = 0;
typedef int(__stdcall* tPreFrameUpdate)(float dt);
tPreFrameUpdate oPreFrameUpdate = 0;

fb::BorderInputNode* g_pBorderInputNode = fb::BorderInputNode::GetInstance();

#pragma endregion Hook
#pragma region

char *buffer=new char[0xff];

float g_pInputBuffers[123];

bool bAimbot = true;
bool AimKeyPressed = false;
bool bAimHead = false;
bool LockOnEme = false;
std::ofstream log_file(
	"E:\\log\\hk.log", std::ios_base::out | std::ios_base::trunc);

int count = 0;
fb::ClientSoldierEntity* LockOn_pEnemySoldier = nullptr;
#pragma endregion Global Vars

void log(const std::string &text)
{

	log_file << text << std::endl;
}
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


void _stdcall Aimbot() {

	float flBulletGrav;
	float flbulletspeed=-2.0f;
	fb::ClientSoldierEntity* ClosestSold = nullptr;
	fb::Vec3 EnemyAimVec;
	fb::ClientSoldierEntity* pEnemySoldier = nullptr;
	
	fb::Main* pMain = fb::Main::GetInstance();
	if (!POINTERCHK(pMain))
		return;

	fb::Client* pClient = pMain->m_pClient;
	if (!POINTERCHK(pClient))
		return;

	fb::ClientGameContext* pGameContext = pClient->m_pGameContext;
	if (!POINTERCHK(pGameContext))
		return;

	fb::PlayerManager* pPlayerMngr = pGameContext->m_pPlayerManager;

	if (!POINTERCHK(pPlayerMngr))
		return;

	fb::ClientPlayer* pLocalPlayer = pPlayerMngr->m_pLocalPlayer;
	if (!POINTERCHK(pLocalPlayer))
		return;

	if (pLocalPlayer->InVehicle())
		return;

	fb::ClientSoldierEntity* pMySoldier = *((fb::ClientSoldierEntity**)  ((intptr_t)pLocalPlayer + 0x14D0));

	if (!POINTERCHK(pMySoldier))
		return;

    
  
	if (!IsAlive(pMySoldier))return;


	fb::SoldierWeaponComponent* pWepComp = pMySoldier->m_pWeaponComponent;
	if (!POINTERCHK(pWepComp))
		return;

	fb::SoldierWeapon* MySW = pWepComp->GetActiveSoldierWeapon();
	if (!POINTERCHK(MySW))
		return;

	fb::SoldierAimingSimulation* aimer
	= MySW->m_authorativeAiming;


	if (!POINTERCHK(aimer))
		return;


	fb::ClientWeapon* MyCSW = MySW->m_pWeapon;
	if (!POINTERCHK(MyCSW))
		return;
	SM::Matrix ShootSpaceMat = MyCSW->m_ShootSpace;

	fb::Vec3 Origin;
	Origin.x = ShootSpaceMat.Translation().x;
	Origin.y = ShootSpaceMat.Translation().y;
	Origin.z = ShootSpaceMat.Translation().z;


	fb::AimAssist* pAimAssist = aimer->m_pFPSAimer;
	if (!POINTERCHK(pAimAssist))
		return;



	fb::WeaponFiring* pWepFiring = *(fb::WeaponFiring**)OFFSET_PPCURRENTWEAPONFIRING;

	if (!POINTERCHK(pWepFiring))
		return;

	fb::PrimaryFire* pFiring = pWepFiring->m_pPrimaryFire;
	if (!POINTERCHK(pFiring))
		return;

	fb::FiringFunctionData* pFFD = pFiring->m_FiringData;
	if (!POINTERCHK(pFFD)) {
		return;
	}
	else {
		flbulletspeed = pFFD->m_ShotConfigData.m_Speed.z;
	}




	fb::BulletEntityData* pBED = pFFD->m_ShotConfigData.m_ProjectileData;
	if (!POINTERCHK(pBED))return;
	flBulletGrav = pBED->m_Gravity;



	if (POINTERCHK(MyCSW->m_pWeaponModifier))
	{
		if (POINTERCHK(MyCSW->m_pWeaponModifier->m_ShotModifier)) {
			flbulletspeed = MyCSW->m_pWeaponModifier->m_ShotModifier->m_InitialSpeed.z;
		}
	}
	//sprintf_s(buffer, 0xff, "Yaw:%f, Pitch:%f ", pAimAssist->m_AimAngles.x, pAimAssist->m_AimAngles.y); log(buffer);
	
	



	
	if (flbulletspeed == -2.f)return;
//	sprintf_s(buffer, 0xff, "v0:%f, G:%f", flbulletspeed, flBulletGrav); log(buffer);
	fb::Vec3* Enemyvectmp = new fb::Vec3;
	float flScreenDistance;

	if (LockOnEme == false
		|| !POINTERCHK(LockOn_pEnemySoldier)) {
		

		float closestdistance = 9999.0f;

		for (int i = 0; i <70; i++)
		{

			fb::ClientPlayer* pClientPlayer = pPlayerMngr->GetPlayerById(i);


			if (!POINTERCHK(pClientPlayer))
				continue;

			if (pLocalPlayer->m_TeamId == pClientPlayer->m_TeamId)continue;
				

			pEnemySoldier = *((fb::ClientSoldierEntity**)  ((intptr_t)pClientPlayer + 0x14D0));

			if (!POINTERCHK(pEnemySoldier))continue;
				
			if (!IsAlive(pEnemySoldier))continue;
		
		void*	corp = *((fb::ClientSoldierEntity**)  ((intptr_t)pClientPlayer + 0x14a8));

		if (POINTERCHK(corp))continue;

			fb::RagdollComponent* pRagdoll = pEnemySoldier->m_pRagdollComponent;

			if (!POINTERCHK(pRagdoll))continue;
				
			if (bAimHead)
			{
				if (!pRagdoll->GetBone(fb::UpdatePoseResultData::Neck, Enemyvectmp)) continue;

			}
			else
			{
				if (!pRagdoll->GetBone(fb::UpdatePoseResultData::Spine2, Enemyvectmp)) continue;
			}


			

			flScreenDistance = DistanceToCrosshair(Origin,*Enemyvectmp, pAimAssist);
		//	sprintf_s(buffer, 0xff, "Yaw:%f, Pitch:%f,  flScreenDistance:%f ", pAimAssist->m_AimAngles.x,pAimAssist->m_AimAngles.y, flScreenDistance); log(buffer);
			if (flScreenDistance < 0)continue;



			if (flScreenDistance < closestdistance)
			{
			
				ClosestSold = pEnemySoldier;
				closestdistance = flScreenDistance;
				EnemyAimVec =* Enemyvectmp;

			}
		}
		

	}
	else {
	
		pEnemySoldier = LockOn_pEnemySoldier;
		


		fb::RagdollComponent* pRagdoll = pEnemySoldier->m_pRagdollComponent;

		if (!POINTERCHK(pRagdoll))return;
		if (bAimHead)
		{
			if (!pRagdoll->GetBone(fb::UpdatePoseResultData::Neck, Enemyvectmp))return;

		}
		else
		{
			if (!pRagdoll->GetBone(fb::UpdatePoseResultData::Spine2, Enemyvectmp)) return;
		}


		if (!POINTERCHK(Enemyvectmp))return;
		ClosestSold = pEnemySoldier;
		EnemyAimVec =* Enemyvectmp;
		
	}

	if (!POINTERCHK(ClosestSold))return;

	fb::Vec3 * vDir = new fb::Vec3;
	DWORD rc;



	//fb::Vec3 myspeed = pMySoldier->GetVelocity();
	fb::Vec3 enemyspeed = ClosestSold->GetVelocity();


	PRINT(sprintf_s(buffer, 0xff, "enemyspeed.x:%f,start aim", enemyspeed.x); log(buffer);)
	


	rc = AimCorrection2(Origin,EnemyAimVec, enemyspeed, flbulletspeed, flBulletGrav, vDir);



	if (!POINTERCHK(vDir)) return;

	if (rc != 0x0){	return;
}

	PRINT(sprintf_s(buffer, 0xff, "fix         yaw:%f,pitch:%f", vDir->x, vDir->y); log(buffer); )



		pAimAssist->m_AimAngles.x = vDir->x-aimer->m_Sway.x;


	pAimAssist->m_AimAngles.y = vDir->y - aimer->m_Sway.y;

	LockOnEme = true;

	LockOn_pEnemySoldier = ClosestSold;
	
}





//minimap spot 
void _stdcall MiniMap() {
	fb::Main* pMain = fb::Main::GetInstance();
	if (!POINTERCHK(pMain))
		return;

	fb::Client* pClient = pMain->m_pClient;
	if (!POINTERCHK(pClient))
		return;

	fb::ClientGameContext* pGameContext = pClient->m_pGameContext;
	if (!POINTERCHK(pGameContext))
		return;

	fb::PlayerManager* pPlayerMngr = pGameContext->m_pPlayerManager;

	if (!POINTERCHK(pPlayerMngr))
		return;

	fb::ClientPlayer* pLocalPlayer = pPlayerMngr->m_pLocalPlayer;
	if (!POINTERCHK(pLocalPlayer))
		return;

	int TeamId = pLocalPlayer->m_TeamId;

	

	fb::ClientSoldierEntity* pLocalSoldier = *((fb::ClientSoldierEntity**)  ((intptr_t)pLocalPlayer + 0x14D0));

	if (!POINTERCHK(pLocalSoldier))
		return;


	if (!IsAlive(pLocalSoldier))return;
	if (count == 0) { PRINT(sprintf_s(buffer, 0xff, "teamid:%d", TeamId); log(buffer);) count = 1; };
	eastl::vector<fb::ClientPlayer*>* pVecCP = pPlayerMngr->getPlayers(); //vector




	

	for (int i = 0; i <70; i++)
	{

		fb::ClientPlayer* pClientPlayer = pPlayerMngr->GetPlayerById(i);

		if (!POINTERCHK(pClientPlayer)) continue;

		if (TeamId == pClientPlayer->m_TeamId)continue;

		if (pClientPlayer->InVehicle())continue;




		fb::ClientSoldierEntity* pSoldier = *(fb::ClientSoldierEntity**)((intptr_t)pClientPlayer + 0x14D0);

		if (!POINTERCHK(pSoldier)) continue;


		fb::ClientSpottingTargetComponent* pCSTC = pSoldier->m_pSpottingComp;

		if (POINTERCHK(pCSTC)) {
			if (pCSTC->activeSpotType == fb::SpotType_None) { pCSTC->activeSpotType = fb::SpotType_Passive; }
			else
			{
				continue;
			}
		}

	}


}
void _stdcall SoldierWeaponUpgrade() {


	//unlock
	LPVOID unlock = *(LPVOID*)OFFSET_SYNCEDBFSETTINGS;
	if (POINTERCHK(unlock)) {

		if (*(bool*)((intptr_t)unlock + 0x54) != true) {
			*(bool*)((intptr_t)unlock + 0x54) = true;
		}
	}

	fb::ClientPlayer* pLocalPlayer = GetLocalPlayer();
	if (!POINTERCHK(pLocalPlayer))
		return;

	if (pLocalPlayer->InVehicle())
		return;

	fb::ClientSoldierEntity* pLocalSoldier = *(fb::ClientSoldierEntity**)((intptr_t)pLocalPlayer + 0x14D0);
	if (!POINTERCHK(pLocalSoldier))
		return;
	if (!IsAlive(pLocalSoldier))return;
	fb::BreathControlHandler *pBreath = *(fb::BreathControlHandler **)((intptr_t)pLocalSoldier + 0x588);

	if (POINTERCHK(pBreath)) {

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

	//if (pLW->m_authorativeAiming->m_zoomLevel > 0)
	//{
	//	if (!POINTERCHK(pLW->m_authorativeAiming->m_data))return;
	//	

	//		if (POINTERCHK(pLW->m_authorativeAiming->m_data->m_ZoomLevels[0]))
	//		{
	//			fb::ZoomLevelData *zoomLevel = pLW->m_authorativeAiming->m_data->m_ZoomLevels[0];
	//			if (POINTERCHK(zoomLevel))
	//			{

	//				zoomLevel->m_SuppressedSwayPitchMagnitude = 0.0f;
	//				zoomLevel->m_SuppressedSwayMinFactor = 0.0f;
	//				zoomLevel->m_SuppressedSwayPitchMagnitude = 0.0f;
	//				zoomLevel->m_SuppressedSwayYawMagnitude = 0.0f;
	//				zoomLevel->m_SwayPitchMagnitude = 0.0f;
	//				zoomLevel->m_SwayYawMagnitude = 0.0f;
	//				//zoomLevel->m_supportedSwayPitchMlt = 0.0f;
	//				//zoomLevel->m_supportedSwayYawMlt = 0.0f;
	//			}
	//		}
	//		if (POINTERCHK(pLW->m_authorativeAiming->m_data->m_ZoomLevels[1]))
	//		{
	//			fb::ZoomLevelData *zoomLevel1 = pLW->m_authorativeAiming->m_data->m_ZoomLevels[1];
	//			if (POINTERCHK(zoomLevel1))
	//			{
	//				zoomLevel1->m_SuppressedSwayPitchMagnitude = 0.0f;
	//				zoomLevel1->m_SuppressedSwayMinFactor = 0.0f;
	//				zoomLevel1->m_SuppressedSwayPitchMagnitude = 0.0f;
	//				zoomLevel1->m_SuppressedSwayYawMagnitude = 0.0f;
	//				zoomLevel1->m_SwayPitchMagnitude = 0.0f;
	//				zoomLevel1->m_SwayYawMagnitude = 0.0f;
	//				//zoomLevel1->m_supportedSwayPitchMlt = 0.0f;
	//				//zoomLevel1->m_supportedSwayYawMlt = 0.0f;
	//			}
	//		
	//	}
	//}
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
	

	int returnval=oPreFrameUpdate(DeltaTime);

	if (GetAsyncKeyState(VK_RMENU) & 0x8000) {
		bAimHead = !bAimHead;

	};
	if (GetAsyncKeyState(VK_LMENU) & 0x8000) {
		AimKeyPressed = true;
	}
	else { AimKeyPressed = false; }

	if (bAimbot &&  AimKeyPressed) {
		Aimbot();
	}
	else {
		LockOnEme = false;
		LockOn_pEnemySoldier = nullptr;
	}

	for (int i = 0; i < 123; i++)
	{
		g_pBorderInputNode->m_InputCache->m_Event[i] += g_pInputBuffers[i];
		g_pInputBuffers[i] = 0.0f;
	}



	return returnval;
}
HRESULT __stdcall hkPresent(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags) {


SoldierWeaponUpgrade();
	MiniMap();





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

BOOL APIENTRY DllMain(HMODULE hModule,
	DWORD  ul_reason_for_call,
	LPVOID lpReserved
)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH: {

		DisableThreadLibraryCalls(hModule);
	





		
	
		
		PreFrameHook = new CVMTHookManager((intptr_t**)fb::BorderInputNode::GetInstance()->m_Vtable);
		oPreFrameUpdate = (tPreFrameUpdate)PreFrameHook->dwGetMethodAddress(3);
		PreFrameHook->dwHookMethod((intptr_t)hkPreFrame, 3);
		
	PRINT(	sprintf_s(buffer, 0xff, "hook PreFrame success"); log(buffer);)

		PresentHook = new CVMTHookManager((intptr_t**)fb::DxRenderer::GetInstance()->m_pScreen->m_pSwapChain);
		oPresent = (tPresent)PresentHook->dwGetMethodAddress(8);
		PresentHook->dwHookMethod((intptr_t)hkPresent, 8);

		PRINT(sprintf_s(buffer, 0xff, "hook Present success"); log(buffer);)


		LPVOID IcmpCreateFile = (LPVOID)((intptr_t)GetProcAddress(GetModuleHandleW(L"iphlpapi.dll"), "IcmpCreateFile") + 0x327);

		DWORD dwOld;
		//ping spoof
		if (POINTERCHK(IcmpCreateFile)) {
			VirtualProtect((LPVOID)IcmpCreateFile, 3 * sizeof(BYTE), PAGE_EXECUTE_READWRITE, &dwOld);

			memset((LPVOID)IcmpCreateFile, 0x31, 1);
			memset((LPVOID)((intptr_t)IcmpCreateFile + 1), 0xc0, 1);//xor eax,eax
			memset((LPVOID)((intptr_t)IcmpCreateFile + 2), 0x90, 1);//inc eax

			VirtualProtect((LPVOID)IcmpCreateFile, 3 * sizeof(BYTE), dwOld, NULL);


		};
		
		PRINT(sprintf_s(buffer, 0xff, "ping spoof"); log(buffer);)
	}break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH: {if (PresentHook != nullptr)PresentHook->UnHook(); 
		if (PreFrameHook != nullptr)PreFrameHook->UnHook();
	
	}
		break;
	}
	return TRUE;
}
