#include "stdafx.h"
#include "FB_SDK/Frostbite.h"
#include "../vc3hk/VMTHook.h"

#include "Aimbot.h"
#include "hkheader.h"

#include <VersionHelpers.h>



//#define LOGGG
char *buffer = new char[0xff];
std::ofstream log_file(
	"E:\\log\\hk.log", std::ios_base::out | std::ios_base::trunc);
void logxxx(const std::string &text)
{

	log_file << text << std::endl;
}

#pragma region

CVMTHookManager* PresentHook = 0;
typedef HRESULT(__stdcall* tPresent)(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags);
tPresent oPresent;


CVMTHookManager* PreFrameHook = 0;
typedef  int(__stdcall* tPreFrameUpdate)(void*, float);
tPreFrameUpdate oPreFrameUpdate = 0;

fb::BorderInputNode* g_pBorderInputNode = fb::BorderInputNode::GetInstance();


CVMTHookManager* updateHook = 0;
typedef DWORD_PTR(__stdcall* tUpdate)(DWORD_PTR a1, DWORD_PTR a2, DWORD_PTR a3);
tUpdate oUpdate;

intptr_t* pAntVtable = *(intptr_t**)OFFSET_ANTVTABLE;
intptr_t* pInputNodeVtable = (intptr_t*)fb::BorderInputNode::GetInstance()->m_Vtable;
#pragma endregion Hook
#pragma region

intptr_t last_weaponFiring = 0;


LARGE_INTEGER l_cur_time;

LARGE_INTEGER l_last_time;





float last_yaw_InputBuffer = 0;
float last_pitch_InputBuffer = 0;
float last_roll_InputBuffer = 0;

float g_pInputBuffers[123] = { 0.f };



bool bAimbot = true;
bool AimKeyPressed = false;
bool bAimHead = false;

bool bAmmoBox = false;
bool b_Unlcok = false;
intptr_t* slotG1_list[4][15] = { 0 };
//intptr_t* slotG2_list[4][15] = { 0 };

Aimbot hkAimbot;







#pragma endregion Global Vars




void AmmoBox(fb::ClientGameContext* p) {
	if (bAmmoBox)return;
	fb::Level* pLevel = p->m_pLevel;

	// POINTERCHK
	if (!POINTERCHK(pLevel)) {

		bAmmoBox = false;	return;
	}

	if (!POINTERCHK(pLevel->m_TeamEntity[1])) {

		bAmmoBox = false;	return;
	};


	if (!POINTERCHK(pLevel->m_TeamEntity[1]->m_Team))
		return;



	intptr_t EndOfArray = 0;

	fb::TeamEntityData* m_teamEntity;

	fb::TeamData* Team;







	for (int i = 1; i <= 2; i++) {

		m_teamEntity = pLevel->m_TeamEntity[i];

		if (!POINTERCHK(m_teamEntity))
			return;

		Team = m_teamEntity->m_Team;


		if (!POINTERCHK(Team))
			return;



		if (!POINTERCHK((void*)(Team->m_SoldierCustomization[0])))
			return;


		if (!POINTERCHK((void*)(Team->m_SoldierCustomization[0]->m_pWeaponTable)))
			return;

		if (!POINTERCHK((void*)(Team->m_SoldierCustomization[0]->m_pWeaponTable->m_ppList[0])))
			return;
		if (!POINTERCHK((void*)(Team->m_SoldierCustomization[0]->m_pWeaponTable->m_ppList[0]->m_SelectableUnlocks[0])))
			return;

		slotG1_list[0][0] = Team->m_SoldierCustomization[0]->m_pWeaponTable->m_ppList[2]->m_SelectableUnlocks[0];
		//	slotG2_list[0][0] = Team->m_SoldierCustomization[0]->m_pWeaponTable->m_ppList[3]->m_SelectableUnlocks[0];
		for (int j = 0; j < 4; j++) {

			int index = 1;

			while (index < 15)
			{
				slotG1_list[j][index] = Team->m_SoldierCustomization[j]->m_pWeaponTable->m_ppList[2]->m_SelectableUnlocks[index];
				//	slotG2_list[j][index] = Team->m_SoldierCustomization[j]->m_pWeaponTable->m_ppList[3]->m_SelectableUnlocks[index];
				index++;
			}




		}
		slotG1_list[3][15] = &EndOfArray;
		//slotG2_list[3][15] = &EndOfArray;      //0  //1  //2 //5 //6 //7
		for (int k = 0; k < 4; k++) {

			Team->m_SoldierCustomization[k]->m_pWeaponTable->m_ppList[7]->m_SelectableUnlocks[2] = slotG1_list[2][6]; //ammobox

			Team->m_SoldierCustomization[k]->m_pWeaponTable->m_ppList[2]->m_SelectableUnlocks[0] = slotG1_list[0][11]; //U_Medkit

			Team->m_SoldierCustomization[k]->m_pWeaponTable->m_ppList[5]->m_SelectableUnlocks[0] = slotG1_list[3][2]; //C4
		}
	}

	bAmmoBox = true;
}




void _stdcall DoAim() {

	float flBulletGrav;
	float zero_angle = 0.f;
	int zero_index = -2;

	fb::Vec4 v_bulletspeed;
	fb::ClassInfo* pType;
	v_bulletspeed.z = -2.f;
	v_bulletspeed.y = -2.f;
	int bulletId = 0;
	bool b_InVehWithWeapon = false;
	bool b_HasTurret = false;
	fb::ChildRotationBodyData * p_turrentData = nullptr;
	fb::Vec4 v_Origin;
	fb::Vec4 * curVecOfClosestSoldier;
	fb::ClientSoldierEntity* p_ClosestSoldier;
	fb::ClientPlayer* p_ClosestPlayer;
	fb::ClientSoldierEntity* pEnemySoldier = nullptr;
	float fl_YawMultiplier = 200.0f;
	float fl_PitchMultiplier = 200.0f;




	fb::ClientGameContext* pGameContext = fb::ClientGameContext::GetInstance();
	if (!POINTERCHK(pGameContext))
		return;

	fb::PlayerManager* pPlayerMngr = pGameContext->m_pPlayerManager;

	if (!POINTERCHK(pPlayerMngr))
		return;

	fb::ClientPlayer* pLocalPlayer = pPlayerMngr->m_pLocalPlayer;
	if (!POINTERCHK(pLocalPlayer))
		return;



	fb::ClientSoldierEntity* pMySoldier = pLocalPlayer->GetSoldierEntity();

	if (!POINTERCHK(pMySoldier))
		return;


	if (!IsAlive(pMySoldier))return;

	void* pCurVehicleCam = *(void**)(OFFSET_PPCURRENTWEAPONFIRING + 0x10);
	if (pLocalPlayer->InVehicle() && POINTERCHK(pCurVehicleCam)) {
		b_InVehWithWeapon = true;
		sprintf_s(buffer, 0xff, "	b_InVehWithWeapon = true"); logxxx(buffer);


	}
	
	fb::ClientWeapon* MyCSW = nullptr;
	fb::WeaponFiring* pWepFiring = *(fb::WeaponFiring**)OFFSET_PPCURRENTWEAPONFIRING;


	if (!b_InVehWithWeapon) {
		fb::ClientSoldierWeaponsComponent* pWepComp = fb::MainVarPtr::Singleton()->pWeaponComp;
		if (!POINTERCHK(pWepComp))
			return; else {
			zero_index = pWepComp->m_ZeroingDistanceLevel;

		}

		fb::ClientSoldierWeapon* MySW = pWepComp->GetActiveSoldierWeapon();
		if (!POINTERCHK(MySW))
			return;

		

		MyCSW = MySW->m_pWeapon;
		pWepFiring = MySW->m_pPrimary;
	}


	if (!POINTERCHK(pWepFiring)) {
		return;
	}
	fb::WeaponFiringData* pFiring = pWepFiring->m_pPrimaryFire;
	if (!POINTERCHK(pFiring))
		return;

	fb::FiringFunctionData* pFFD = pFiring->m_FiringData;
	if (!POINTERCHK(pFFD)) {
		return;
	}
	else {
		v_bulletspeed.z = pFFD->m_ShotConfigData.m_InitialSpeed.z;
		v_bulletspeed.y = pFFD->m_ShotConfigData.m_InitialSpeed.y;
	}

	

	
	





	if (b_InVehWithWeapon) {
		fb::ClientVehicleEntity*	pVehivle = fb::MainVarPtr::Singleton()->pVehicleEntry;

		std::vector<void*>* v_pChildRota
			= pVehivle->GetClientComponentByID(354, true);


		if (POINTERCHK(v_pChildRota)) {
			b_HasTurret = true;


			size_t size = v_pChildRota->size() < 2 ? v_pChildRota->size() : 2;

			for (size_t index = 0; index < size; index++)
			{
				fb::ChildRotationBodyData*	p_ChildRotationBodyData =
					((fb::ClientChildComponent*)v_pChildRota->at(index))
					->m_ClientChildBarrelComponent.m_ChildRotationBodyData;


				if (POINTERCHK(p_ChildRotationBodyData)) {
					if (p_ChildRotationBodyData->m_RotationAxis == 1) {

						fl_YawMultiplier = p_ChildRotationBodyData->m_AngularMomentumMultiplier;

					}
					else	if (p_ChildRotationBodyData->m_RotationAxis == 0) {

						fl_PitchMultiplier = p_ChildRotationBodyData->m_AngularMomentumMultiplier;

					}


				}




			}

		}
		delete v_pChildRota;
		v_pChildRota = nullptr;


	}else{

		if (!POINTERCHK(MyCSW))
			return;

		if (POINTERCHK(MyCSW->m_pWeaponModifier))
		{
			if (POINTERCHK(MyCSW->m_pWeaponModifier->m_ZeroingModifier) && zero_index >= 0) {

				zero_angle = MyCSW->m_pWeaponModifier->m_ZeroingModifier->GetZeroLevelAt(zero_index).m_Angle;
			}
			if (POINTERCHK(MyCSW->m_pWeaponModifier->m_ShotModifier)) {

				v_bulletspeed.z = MyCSW->m_pWeaponModifier->m_ShotModifier->m_InitialSpeed.z;
				v_bulletspeed.y = MyCSW->m_pWeaponModifier->m_ShotModifier->m_InitialSpeed.y;


			}


		}
	}


	void* pBED = pFFD->m_ShotConfigData.m_ProjectileData;
	if (!POINTERCHK(pBED))return;

	//check bullet or missile
	pType = (fb::ClassInfo*)(((fb::ITypedObject*)(pBED))->GetType());

	if (!POINTERCHK(pType))return;

	bulletId = pType->m_ClassId;

	switch (bulletId)
	{
	case 2441: {//missile

		flBulletGrav = *(float*)((intptr_t)pBED + 0x170);
		break;
	};
			   //	case 2434: 
	default: {flBulletGrav = *(float*)((intptr_t)pBED + 0x130); break; };
	}



	if (v_bulletspeed.z <= 10.f)return;
	sprintf_s(buffer, 0xff, "GetInfo ok"); logxxx(buffer);
	//null var
	//hkAimbot.NullTmpVar();


	//get origin


	v_Origin = hkAimbot.GetOriginAndUpdateCurrentAngle(pLocalPlayer, MyCSW, pFFD, b_InVehWithWeapon);
	sprintf_s(buffer, 0xff, "GetOriginAndUpdateCurrentAngle ok"); logxxx(buffer);


	//find closest player


	eastl::vector<fb::ClientPlayer*>* pVecCP = pPlayerMngr->getPlayers();

	curVecOfClosestSoldier = hkAimbot.GetClosestPlayer(pVecCP, pLocalPlayer, bulletId, b_InVehWithWeapon);
	sprintf_s(buffer, 0xff, "find closest soldier ok"); logxxx(buffer);

	p_ClosestSoldier = hkAimbot.mp_ClosestSoldier;
	p_ClosestPlayer = hkAimbot.mp_ClosestPlayer;
	if (!POINTERCHK(p_ClosestSoldier))return;
	if (!POINTERCHK(p_ClosestPlayer))return;
	if (!POINTERCHK(curVecOfClosestSoldier))return;
	





	fb::Vec4 Vec_Enemyspeed;

	Vec_Enemyspeed.x = 0.f;
	Vec_Enemyspeed.y = 0.f;
	Vec_Enemyspeed.z = 0.f;




	if (POINTERCHK(p_ClosestPlayer->m_pAttachedControllable)) {
		sprintf_s(buffer, 0xff, "GetVecVeh"); logxxx(buffer);
		if ((*(intptr_t*)(p_ClosestPlayer->m_pAttachedControllable))!=0x141BCC140)goto OnFoot;
	

	

		 fb::Vec4 * v = ((fb::ClientVehicleEntity *)p_ClosestPlayer->m_pAttachedControllable)->GetVehicleSpeed();

		 if (POINTERCHK(v)) {
			 Vec_Enemyspeed = *v;
			 sprintf_s(buffer, 0xff, "v_eh_Vec_Enemyspeed.x:%f\tVec_Enemyspeed.y:%f\tVec_Enemyspeed.z:%f",
				 Vec_Enemyspeed.x, Vec_Enemyspeed.y, Vec_Enemyspeed.z); logxxx(buffer);
		 }
	}

	else {
	
		sprintf_s(buffer, 0xff, "GetVecSol"); logxxx(buffer);
		
		fb::Vec4 * v = p_ClosestSoldier->GetSoldierVelocity();
		if (POINTERCHK(v)) {

			Vec_Enemyspeed = *v; sprintf_s(buffer, 0xff, "sol_Enemyspeed.x:%f\tVec_Enemyspeed.y:%f\tVec_Enemyspeed.z:%f",
				Vec_Enemyspeed.x, Vec_Enemyspeed.y, Vec_Enemyspeed.z); logxxx(buffer);
		}

	};
OnFoot:
	Vec_Enemyspeed.w = 0.f;
	sprintf_s(buffer, 0xff, "GetVec ok"); logxxx(buffer);

	fb::Vec4  vDir;
	DWORD n_ReturnCode;


	n_ReturnCode = hkAimbot.AimCorrection2(v_Origin, *curVecOfClosestSoldier, Vec_Enemyspeed, v_bulletspeed, flBulletGrav, &vDir);


	if (n_ReturnCode != 0x0) {
		return;
	}

	sprintf_s(buffer, 0xff, "AimCorrection2_OK"); logxxx(buffer);

	if(zero_index>-1)vDir.y = vDir.y - zero_angle*DegToRad;

	float yawDiff = vDir.x - hkAimbot.v_curAngle.x;
	float pitchDiff = vDir.y - hkAimbot.v_curAngle.y;



	if (abs(yawDiff) > 0.8f || abs(pitchDiff) > 0.8f)return;





	if (!b_InVehWithWeapon) {



		g_pInputBuffers[fb::ConceptYaw] = 0.04f*yawDiff;

		g_pInputBuffers[fb::ConceptPitch] = 0.0225f*pitchDiff;


	}
	else if (b_HasTurret) {


		g_pInputBuffers[fb::ConceptRoll] = 0.003f*fl_YawMultiplier*yawDiff;

		g_pInputBuffers[fb::ConceptPitch] = 0.003f*fl_PitchMultiplier*pitchDiff;



	}
	else { return; }

	hkAimbot.LockOnEnemyFlags = 1;

	hkAimbot.LockOn_pEnemySoldier = p_ClosestSoldier;

}





//minimap spot 
void _stdcall MiniMap(fb::PlayerManager* pPlayerMngr) {

	fb::ClientSoldierWeaponsComponent* pWepComp = fb::MainVarPtr::Singleton()->pWeaponComp;

	if (!POINTERCHK(pWepComp))
		return;
	if (!POINTERCHK(pPlayerMngr))
		return;

	fb::ClientPlayer* pLocalPlayer = pPlayerMngr->m_pLocalPlayer;
	if (!POINTERCHK(pLocalPlayer))
		return;



	fb::ClientSoldierEntity* pLocalSoldier = pLocalPlayer->GetSoldierEntity();

	if (!POINTERCHK(pLocalSoldier))
		return;


	if (!IsAlive(pLocalSoldier))return;



	eastl::vector<fb::ClientPlayer*>* pVecCP = pPlayerMngr->getPlayers();
	if (pVecCP->size() == 0) return;



	for (int i = 0; i < pVecCP->size(); i++)
	{

		fb::ClientPlayer* pClientPlayer = pVecCP->at(i);

		if (!POINTERCHK(pClientPlayer)) continue;

		if (pLocalPlayer->m_TeamId == pClientPlayer->m_TeamId)continue;

		fb::ClientControllableEntity* pControllable;
		if (pClientPlayer->InVehicle()) {
			pControllable = pClientPlayer->m_pAttachedControllable;
		}
		else {
			pControllable = pClientPlayer->m_pControlledControllable;
		}



		if (!POINTERCHK(pControllable)) continue;





		std::vector<void*> *
			v_pCSTC = pControllable->GetClientComponentByID(366, false);


		if (!POINTERCHK(v_pCSTC)) continue;




		fb::ClientSpottingTargetComponent* pCSTC;


		for (int index = 0; index < v_pCSTC->size(); index++)
		{
			pCSTC = (fb::ClientSpottingTargetComponent*)v_pCSTC->at(index);

			if (POINTERCHK(pCSTC)) {
				if (pCSTC->activeSpotType == fb::SpotType_None) { pCSTC->activeSpotType = fb::SpotType_Passive; }
			}



		}

		delete v_pCSTC;

		v_pCSTC = nullptr;

	};





}
void _stdcall SoldierWeaponUpgrade() {


	//unlock

	if (!b_Unlcok) {
		LPVOID unlock = *(LPVOID*)OFFSET_SYNCEDBFSETTINGS;

		if (POINTERCHK(unlock)) {

			*(bool*)((intptr_t)unlock + 0x54) = true;
			b_Unlcok = true;
		}
		else {

			b_Unlcok = false;
		}

	}


	fb::ClientGameContext* pGameContext = fb::ClientGameContext::GetInstance();
	if (!POINTERCHK(pGameContext))
		return;



	fb::PlayerManager* pPlayerMngr = pGameContext->m_pPlayerManager;

	if (!POINTERCHK(pPlayerMngr)) {
	return;
}




	fb::ClientPlayer* pLocalPlayer = pPlayerMngr->m_pLocalPlayer;
	if (!POINTERCHK(pLocalPlayer))
		return;

	MiniMap(pPlayerMngr);
	void* pCurVehicleCam = *(void**)(OFFSET_PPCURRENTWEAPONFIRING + 0x10);

	if ((pLocalPlayer->InVehicle() && POINTERCHK(pCurVehicleCam)))	return;


	AmmoBox(pGameContext);


	fb::ClientSoldierEntity* pLocalSoldier = pLocalPlayer->GetSoldierEntity();

	if (!POINTERCHK(pLocalSoldier)) {
		bAmmoBox = false;	b_Unlcok = false;	return;
	}

	if (!IsAlive(pLocalSoldier))return;

	fb::ClientSoldierWeaponsComponent* pWepComp = fb::MainVarPtr::Singleton()->pWeaponComp;

	if (!POINTERCHK(pWepComp))
		return;

	fb::ClientSoldierWeapon* MySW = pWepComp->GetActiveSoldierWeapon();
	if (!POINTERCHK(MySW))
		return;

	fb::WeaponFiring* pWepFiring;




		pWepFiring = MySW->m_pPrimary;


	if (!POINTERCHK(pWepFiring))
		return;

	fb::WeaponFiringData* pFiring = pWepFiring->m_pPrimaryFire;
	if (!POINTERCHK(pFiring))
		return;

	fb::FiringFunctionData* pFFD = pFiring->m_FiringData;

	if (!POINTERCHK(pFFD)) {
		return;
	}

	if (pFFD->m_ShotConfigData.m_InitialSpeed.z < 10.f)return;


	fb::WeaponSway* pSway = pWepFiring->m_Sway;
	if (!POINTERCHK(pSway))
		return;

	fb::GunSwayData* pSwayData = pSway->m_Data;
	if (!POINTERCHK(pSwayData))
		return;

	//	memset((void*)0x1409A5354,'\x90, 4);
	//	memset((void*)0x1409A535C,'\x90, 2);
	fb::BreathControlHandler *pBreath = *(fb::BreathControlHandler **)((intptr_t)pLocalSoldier + 0x588);

	if (POINTERCHK(pBreath)) {
		pBreath->m_breathControlTimer = 0.f;
		pBreath->m_breathControlMultiplier = 0.f;
		pBreath->m_breathControlPenaltyTimer = 0.f;
		pBreath->m_breathControlpenaltyMultiplier = 0.f;
		pBreath->m_breathControlActive = 0.f;
		pBreath->m_breathControlInput = 0.f;
		pBreath->m_Enabled = 0;

	}
	pSwayData->m_DeviationScaleFactorZoom = 0.f;
	pSwayData->m_GameplayDeviationScaleFactorZoom = 0.f;
	pSwayData->m_DeviationScaleFactorNoZoom = 0.f;
	pSwayData->m_GameplayDeviationScaleFactorNoZoom = 0.f;
	pSwayData->m_FirstShotRecoilMultiplier = 0.f;
	pSwayData->m_ShootingRecoilDecreaseScale = INFINITY;

}



DWORD_PTR hkUpdate(DWORD_PTR a1, DWORD_PTR a2, DWORD_PTR a3)//ICY
{

	/*if (a1)
		*(bool*)(a1 + 0xEC) = true;

	if (a2)
		*(bool*)(a2 + 0x8A) = true;*/

	return oUpdate(a1, a2, a3);
}
int  __stdcall  hkPreFrame(void* ptr, float DeltaTime)
{
	SoldierWeaponUpgrade();

	if (GetAsyncKeyState(0x56) & 0x8000) {
		bAimHead = true;

	}
	else { bAimHead = false; }

	if ((GetAsyncKeyState(VK_LMENU) & 0x8000) || (GetAsyncKeyState(0x56) & 0x8000)) {
		AimKeyPressed = true;
	}
	else {
		AimKeyPressed = false;
	}


	if (bAimbot &&  AimKeyPressed) {

		DoAim();
	}
	else {
		hkAimbot.LockOnEnemyFlags = 0;
		hkAimbot.LockOn_pEnemySoldier = nullptr;
	}



	if (AimKeyPressed) {




		for (int i = 0; i < 20; i++)
		{
			if (isnormal(g_pInputBuffers[i])) {

				g_pBorderInputNode->m_InputCache->m_Event[i] = g_pInputBuffers[i];
			}
			g_pInputBuffers[i] = 0.0f;

		}


	}

	return  oPreFrameUpdate(ptr, DeltaTime);
}
HRESULT __stdcall hkPresent(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags) {


	

	return oPresent(thisptr, SyncInterval, Flags);
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
		DWORD old;
		PreFrameHook = new CVMTHookManager((intptr_t**)fb::BorderInputNode::GetInstance()->m_Vtable);
		oPreFrameUpdate = (tPreFrameUpdate)PreFrameHook->dwGetMethodAddress(3);
		PreFrameHook->dwHookMethod((intptr_t)hkPreFrame, 3);

		//VirtualProtect((void*)(pInputNodeVtable), sizeof(void*), PAGE_EXECUTE_READWRITE, &old);

		//*(intptr_t*)((intptr_t)pInputNodeVtable + 3 * 8) = (intptr_t)hkPreFrame;

		//VirtualProtect((void*)(pInputNodeVtable), sizeof(void*), old, nullptr);




		PresentHook = new CVMTHookManager((intptr_t**)fb::DxRenderer::GetInstance()->m_pScreen->m_pSwapChain);
		oPresent = (tPresent)PresentHook->dwGetMethodAddress(8);
		PresentHook->dwHookMethod((intptr_t)hkPresent, 8);



		VirtualProtect((void*)((intptr_t)pAntVtable + 11 * 8), sizeof(void*), PAGE_EXECUTE_READWRITE, &old);

	//	*(intptr_t*)((intptr_t)pAntVtable + 11 * 8) = (intptr_t)hkUpdate;

		VirtualProtect((void*)((intptr_t)pAntVtable + 11 * 8), sizeof(void*), old, nullptr);

		//printf_s("hook!");
//		sprintf_s(buffer, 0xff, "Hook"); logxxx(buffer);


		//	LPVOID IcmpCreateFile = (LPVOID)((intptr_t)GetProcAddress(GetModuleHandleW(L"iphlpapi.dll"), "IcmpCreateFile") + 0x32a);

			//	char Hijack_spoof_1[13] = { '\xb8','\x11','\x00','\x00','\x00','\x89' ,'\x43','\x08' ,'\xe9' ,'\x74' ,'\xff','\xff' ,'\xff' };
			//	char Hijack_spoof_2[6] = { '\xe9','\x80','\x00','\x00' ,'\x00','\x90'};

				//ping spoof
			//if (POINTERCHK(IcmpCreateFile)) {

			//	DWORD dwOld;



			//	IcmpCreateFile = (LPVOID)((intptr_t)IcmpCreateFile + 0x85);

			//	VirtualProtect((LPVOID)IcmpCreateFile, 13, PAGE_EXECUTE_READWRITE, &dwOld);


			//	memset((LPVOID)IcmpCreateFile, 0xb8, 1);

			//	memset((LPVOID)((intptr_t)IcmpCreateFile + 1), 0x11, 1);//ping ms

			//	memset((LPVOID)((intptr_t)IcmpCreateFile + 2), 0x00, 3);//



			//	memset((LPVOID)((intptr_t)IcmpCreateFile + 5), 0x89, 1);
			//	memset((LPVOID)((intptr_t)IcmpCreateFile + 6), 0x43, 1);
			//	memset((LPVOID)((intptr_t)IcmpCreateFile + 7), 0x08, 1);


			//	memset((LPVOID)((intptr_t)IcmpCreateFile + 8), 0xe9, 1);
			//	memset((LPVOID)((intptr_t)IcmpCreateFile + 9), 0x74, 1);
			//	memset((LPVOID)((intptr_t)IcmpCreateFile + 10), 0xff, 3);


			//	VirtualProtect((LPVOID)IcmpCreateFile, 13, dwOld, NULL);



			//	IcmpCreateFile = (LPVOID)((intptr_t)IcmpCreateFile - 0x85);


			//	VirtualProtect((LPVOID)IcmpCreateFile, 6, PAGE_EXECUTE_READWRITE, &dwOld);

			//	memset((LPVOID)IcmpCreateFile, 0xe9, 1);
			//	memset((LPVOID)((intptr_t)IcmpCreateFile + 1), 0x80, 1);
			//	memset((LPVOID)((intptr_t)IcmpCreateFile + 2), 0x00, 3);

			//	memset((LPVOID)((intptr_t)IcmpCreateFile + 5), 0x90, 1);


			//	VirtualProtect((LPVOID)IcmpCreateFile, 6, dwOld, NULL);
			//};


	}; break;
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH: {

		if (PresentHook != nullptr)PresentHook->UnHook();
		if (PreFrameHook != nullptr)PreFrameHook->UnHook();
		log_file.close();
	}
							 break;
	}
	return TRUE;
}
