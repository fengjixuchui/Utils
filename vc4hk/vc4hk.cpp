// vc4hk.cpp : 定义 DLL 应用程序的导出函数。
//

#include "stdafx.h"
#include "../vc3hk/VMTHook.h"

#include "x64_sdk.h"
#include "Functions4.h"
#include <VersionHelpers.h>

using namespace fb;

//#define LOGGG

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
typedef signed int(__stdcall* tPreFrameUpdate)(float dt);
tPreFrameUpdate oPreFrameUpdate = 0;

fb::BorderInputNode* g_pBorderInputNode = fb::BorderInputNode::GetInstance();


CVMTHookManager* updateHook = 0;
typedef DWORD_PTR(__stdcall* tUpdate)(DWORD_PTR a1, DWORD_PTR a2, DWORD_PTR a3);
tUpdate oUpdate;

intptr_t* pAntVtable = *(intptr_t**)OFFSET_ANTVTABLE;
intptr_t* pInputNodeVtable = (intptr_t*)fb::BorderInputNode::GetInstance()->m_Vtable;
#pragma endregion Hook
#pragma region
Vec3 v_curAngle;
SM::Vector3 v_oriUp(0.f, 1.f, 0.f);
Vec3 v_oriRight(1.f, 0.f, 0.f, 0.f);


float g_pInputBuffers[123] = { 0.f };

bool bAimbot = true;
bool AimKeyPressed = false;
bool bAimHead = false;
bool LockOnEme = false;
bool bAmmoBox = false;
intptr_t* slotG1_list[4][15] = { 0 };
//intptr_t* slotG2_list[4][15] = { 0 };
int count = 0;
fb::ClientSoldierEntity* LockOn_pEnemySoldier = nullptr;
#pragma endregion Global Vars




void AmmoBox(ClientGameContext* p) {

	Level* pLevel = p->m_pLevel;

	// POINTERCHK
	if (!POINTERCHK(pLevel)) {

		bAmmoBox = false;	return;
	}

	if (!POINTERCHK(pLevel->m_TeamEntity[1])) {

		bAmmoBox = false;	return;
	};


	if (!POINTERCHK(pLevel->m_TeamEntity[1]->m_Team))
		return;
	if (bAmmoBox)return;

	intptr_t EndOfArray = 0;

	TeamEntityData* m_teamEntity;

	TeamData* Team;







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




void _stdcall Aimbot() {

	float flBulletGrav;
	float zero_angle = 0.f;
	int zero_index = -2;
	float closestdistance = 9999.0f;
	fb::Vec3 flbulletspeed;
	fb::ClassInfo* pType;
	flbulletspeed.z = -2.f;
	int ClassId = 0;
	bool found = false;

	Vec3 vOrigin;




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

//	if (pLocalPlayer->InVehicle())return;

	fb::ClientSoldierEntity* pMySoldier = pLocalPlayer->GetSoldierEntity();

	if (!POINTERCHK(pMySoldier))
		return;


	if (!IsAlive(pMySoldier))return;


	fb::SoldierWeaponComponent* pWepComp = *(fb::SoldierWeaponComponent **)((intptr_t)pMySoldier + 0x570);
	if (!POINTERCHK(pWepComp))
		return; else {
		zero_index = pWepComp->m_ZeroingDistanceLevel;

	}

	fb::SoldierWeapon* MySW = pWepComp->GetActiveSoldierWeapon();
	if (!POINTERCHK(MySW))
		return;

	fb::ClientWeapon* MyCSW = MySW->m_pWeapon;
	if (!POINTERCHK(MyCSW))
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
		flbulletspeed.z = pFFD->m_ShotConfigData.m_InitialSpeed.z;
		flbulletspeed.y = pFFD->m_ShotConfigData.m_InitialSpeed.y;
	}

	void* pBED = pFFD->m_ShotConfigData.m_ProjectileData;

	if (!POINTERCHK(pBED))return;

	//check bullet or missile
	pType = (fb::ClassInfo*)(((fb::ITypedObject*)(pBED))->GetType());

	if (!POINTERCHK(pType))return;

	ClassId = pType->m_ClassId;

	switch (ClassId)
	{
	case 2441: {//missile

		flBulletGrav = *(float*)((intptr_t)pBED + 0x170);
		break;
	}; 
//	case 2434: 
	default: {flBulletGrav = *(float*)((intptr_t)pBED + 0x130); break; };
	}


	//sprintf_s(buffer, 0xff, "4"); log(buffer);



	if (POINTERCHK(MyCSW->m_pWeaponModifier))
	{
		if (POINTERCHK(MyCSW->m_pWeaponModifier->m_ZeroingModifier) && zero_index >= 0) {

			zero_angle = MyCSW->m_pWeaponModifier->m_ZeroingModifier->GetZeroLevelAt(zero_index).m_Angle;
		}
		if (POINTERCHK(MyCSW->m_pWeaponModifier->m_ShotModifier)) {

			flbulletspeed.z = MyCSW->m_pWeaponModifier->m_ShotModifier->m_InitialSpeed.z;
			if (zero_index < 0)	flbulletspeed.y = MyCSW->m_pWeaponModifier->m_ShotModifier->m_InitialSpeed.y;


		}


	}



	if (flbulletspeed.z == -2.f)return;


	

	//	sprintf_s(buffer,0xff, "G:%f", flBulletGrav); log(buffer);



	SM::Matrix ShootSpaceMat = MyCSW->m_ShootSpace;

	v_curAngle.x = -atan2(ShootSpaceMat._31, ShootSpaceMat._33);

	if (v_curAngle.x < 0) { v_curAngle.x = v_curAngle.x + Twice_PI; }


	if (pLocalPlayer->InVehicle()) {
		SM::Vector3 tmp;
		CUR_turrent* turrent = CUR_turrent::Singleton();
		SM::Matrix *mTransform = &turrent->m_turretTransform;

		if (!POINTERCHK(mTransform))return;

		tmp = mTransform->Backward();
		v_curAngle.y = atan2(tmp.y, sqrt(tmp.x*tmp.x + tmp.z*tmp.z));
		
	}
	else {
		v_curAngle.y = atan2(ShootSpaceMat._32, sqrt(ShootSpaceMat._33* ShootSpaceMat._33 + ShootSpaceMat._31* ShootSpaceMat._31));
	}
	/*if (pLocalPlayer->InVehicle()) {
		SM::Matrix *mTransform = new SM::Matrix;

		if (POINTERCHK(pLocalPlayer->GetVehicleEntity())) {
		pLocalPlayer->GetVehicleEntity()->GetTransform(mTransform);


		tmp.x = 0.f; tmp.y =0.f; tmp.z = 1.f;

		tmp.x = tmp.x*mTransform->_11 + tmp.y*mTransform->_21 + tmp.z*mTransform->_31;
		tmp.y = tmp.x*mTransform->_12 + tmp.y*mTransform->_22 + tmp.z*mTransform->_32;
		tmp.z = tmp.x*mTransform->_13 + tmp.y*mTransform->_23 + tmp.z*mTransform->_33;



		v_curAngle.y = atan2(tmp.y, sqrt(tmp.x*tmp.x + tmp.z*tmp.z));


	}
	}*/

	//v_curAngle.y = v_curAngle.y*180 / M_PI;
	//v_curAngle.x = v_curAngle.x*180 / M_PI;
	//v_curAngle.z = v_curAngle.z*180 / M_PI;
	//sprintf_s(buffer, 0xff, "Y:%f,P:%f,R:%f", v_curAngle.x, v_curAngle.y,v_curAngle.z); log(buffer);
	//	return;


	vOrigin.x = ShootSpaceMat.Translation().x + pFFD->m_ShotConfigData.m_InitialPosition.x;
	vOrigin.y = ShootSpaceMat.Translation().y + pFFD->m_ShotConfigData.m_InitialPosition.y;
	vOrigin.z = ShootSpaceMat.Translation().z + pFFD->m_ShotConfigData.m_InitialPosition.z;





	eastl::vector<fb::ClientPlayer*>* pVecCP = pPlayerMngr->getPlayers();
	if (pVecCP->size() == 0) return;
	fb::Vec3* Enemyvectmp = new fb::Vec3;

	float flScreenDistance;

	fb::ClientPlayer* pClientPlayer = nullptr;


	if (LockOnEme == false || !POINTERCHK(LockOn_pEnemySoldier)) {


		for (int i = 0; i < pVecCP->size(); i++)
		{

			pClientPlayer = pVecCP->at(i);


			if (!POINTERCHK(pClientPlayer))
				continue;

			if (pLocalPlayer->m_TeamId == pClientPlayer->m_TeamId)continue;


			pEnemySoldier = pClientPlayer->GetSoldierEntity();


			if (!POINTERCHK(pEnemySoldier))continue;

			if (!IsAlive(pEnemySoldier))continue;

			if (!pClientPlayer->InVehicle() && pEnemySoldier->m_Occluded == 1)continue;

			fb::RagdollComponent* pRagdoll = *(fb::RagdollComponent**)((intptr_t)pEnemySoldier + 0x580);

			if (!POINTERCHK(pRagdoll))continue;

			if (bAimHead)
			{
				found = pRagdoll->GetBone(fb::UpdatePoseResultData::Neck, Enemyvectmp);
			}

			else
			{
				found = pRagdoll->GetBone(fb::UpdatePoseResultData::Spine2, Enemyvectmp);
			}




			if (!found) {

				if ((pClientPlayer->InVehicle() && ClassId == 2441) || pLocalPlayer->InVehicle()) {
					if (GetVectorFromeVehicle(pClientPlayer, Enemyvectmp) != 0)continue;


				}

				else {
					continue;
				}

			}

			if (!POINTERCHK(Enemyvectmp))continue;

			flScreenDistance = DistanceToCrosshair(vOrigin, *Enemyvectmp, v_curAngle);


			if (flScreenDistance < 0)continue;




			if (flScreenDistance < closestdistance)
			{

				ClosestSold = pEnemySoldier;
				closestdistance = flScreenDistance;
				EnemyAimVec = *Enemyvectmp;

			}

		}


	}
	else {

		pEnemySoldier = LockOn_pEnemySoldier;

		if (!POINTERCHK(pEnemySoldier))return;

		if (!IsAlive(pEnemySoldier))return;

		if (!pEnemySoldier->m_pPlayer->InVehicle()&&pEnemySoldier->m_Occluded==1)return;

		fb::RagdollComponent* pRagdoll = *(fb::RagdollComponent**)((intptr_t)pEnemySoldier + 0x580);

		if (!POINTERCHK(pRagdoll))return;

		if (bAimHead)
		{
			found = pRagdoll->GetBone(fb::UpdatePoseResultData::Neck, Enemyvectmp);



		}

		else
		{
			found = pRagdoll->GetBone(fb::UpdatePoseResultData::Spine2, Enemyvectmp);



		}


		pClientPlayer = pEnemySoldier->m_pPlayer;

		if (!POINTERCHK(pClientPlayer))return;
		if (!found) {

			if ((pClientPlayer->InVehicle() && ClassId == 2441) || pLocalPlayer->InVehicle()) {


				if (GetVectorFromeVehicle(pClientPlayer, Enemyvectmp) != 0)return;


			}




			else {
				return;
			}
		}

		if (!POINTERCHK(Enemyvectmp))return;

		ClosestSold = pEnemySoldier;
		EnemyAimVec = *Enemyvectmp;

	}



	if (!POINTERCHK(ClosestSold))return;

	fb::Vec3 * vDir = new fb::Vec3;
	DWORD rc;

	fb::Vec3 enemyspeed;

	enemyspeed.x = 0.f;
	enemyspeed.y = 0.f;
	enemyspeed.z = 0.f;


	if (!IsAlive(ClosestSold))return;

	if (ClosestSold->m_pPlayer->InVehicle()) {
		enemyspeed = getVehicleSpeed(ClosestSold);
	}

	else {
		enemyspeed = ClosestSold->GetVelocity();
	};
	//sprintf_s(buffer, 0xff, "AimCorrection2"); log(buffer);
	rc = AimCorrection2(vOrigin, EnemyAimVec, enemyspeed, flbulletspeed, flBulletGrav, vDir);



	if (!POINTERCHK(vDir)) return;

	if (rc != 0x0) {
		return;
	}



//	sprintf_s(buffer, 0xff, "vDir"); log(buffer);

	vDir->y = vDir->y - zero_angle*DegToRad;
	//pAimAssist->m_AimAngles.x = vDir->x;//yaw

	if (abs(vDir->x - v_curAngle.x) > 1.05f || abs(vDir->y - v_curAngle.y) > 1.f)return;




	//pAimAssist->m_AimAngles.y = ;  //pitch
	if (!pLocalPlayer->InVehicle()
		) {
		g_pInputBuffers[fb::ConceptYaw] = (vDir->x - v_curAngle.x)* 0.020000f;
		g_pInputBuffers[fb::ConceptPitch] = (vDir->y - v_curAngle.y)*0.020000f;
		//g_pInputBuffers[fb::ConceptRoll] = (vDir->x - v_curAngle.x)*0.040000f;
	}
	else {


		//g_pInputBuffers[fb::ConceptYaw] = (vDir->x - v_curAngle.x)* 2.5f;
		g_pInputBuffers[fb::ConceptPitch] = (vDir->y - v_curAngle.y)*4.f;
		g_pInputBuffers[fb::ConceptRoll] = (vDir->x - v_curAngle.x)* 4.f;



	}
	LockOnEme = true;

	LockOn_pEnemySoldier = ClosestSold;

}





//minimap spot 
void _stdcall MiniMap(fb::PlayerManager* pPlayerMngr) {


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

		fb::ClientControllableEntity* pSoldier;
		 pSoldier=pClientPlayer->GetSoldierEntity(); 
		






		if (!POINTERCHK(pSoldier)) continue;
		


		if (!IsAlive(pSoldier))continue;

		fb::ClientSpottingTargetComponent* pCSTC;
		if (!pClientPlayer->InVehicle()) {

		 pCSTC = pSoldier->GetClientSpottingTargetComponent();

		
		 if (pCSTC->activeSpotType == 0 ) { pCSTC->activeSpotType = fb::SpotType_Active; }

		}
		else
		{
			continue;
			
		/*	for (int offset = 0x0500; offset <= 0x06F0; offset += 0x8)
			{

				pCSTC = (fb::ClientSpottingTargetComponent*)((intptr_t)pSoldier + offset);

				if (POINTERCHK(pCSTC))
				{
					fb::ClientSpottingTargetComponent::SpottingTargetComponentData* pSpottingTargetComponentData = pCSTC->m_spottingTargetData;

					if (POINTERCHK(pSpottingTargetComponentData))
					{

						float ActiveSpottedTime = pSpottingTargetComponentData->m_ActiveSpottedTime;
						float PassiveSpottedTime = pSpottingTargetComponentData->m_PassiveSpottedTime;

						if (ActiveSpottedTime == 12 && PassiveSpottedTime == 1.5)
						{
							if (pCSTC->activeSpotType == 0)
							{ pCSTC->activeSpotType = fb::SpotType_Active; }


						}
						else continue;
					}
				}else continue;
			}*/
		
			}; 
		

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

	MiniMap(pPlayerMngr);


	fb::ClientPlayer* pLocalPlayer = pPlayerMngr->m_pLocalPlayer;
	if (!POINTERCHK(pLocalPlayer))
		return;
	AmmoBox(pGameContext);




	fb::ClientSoldierEntity* pLocalSoldier = pLocalPlayer->GetSoldierEntity();
	if (!POINTERCHK(pLocalSoldier)) {
		bAmmoBox = false;	return;
	}

	if (!IsAlive(pLocalSoldier))return;

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


	//fb::SoldierWeaponComponent* pWepComp = *(fb::SoldierWeaponComponent **)((intptr_t)pLocalSoldier + 0x570);
	//if (!POINTERCHK(pWepComp))
	//	return;
	//if (pWepComp->m_CurrentWeaponIndex != 0 && pWepComp->m_CurrentWeaponIndex != 1)return;
	//fb::SoldierWeapon* pLW = pWepComp->GetActiveSoldierWeapon();
	//if (!POINTERCHK(pLW))
	//	return;
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

	if (pFFD->m_ShotConfigData.m_InitialSpeed.z < 15.f)return;




	fb::WeaponSway* pSway = pWepFiring->m_Sway;
	if (!POINTERCHK(pSway))
		return;

	fb::GunSwayData* pSwayData = pSway->m_Data;
	if (!POINTERCHK(pSwayData))
		return;

	//	memset((void*)0x1409A5354,'\x90, 4);
	//	memset((void*)0x1409A535C,'\x90, 2);

	pSwayData->m_DeviationScaleFactorZoom = 0.f;
	pSwayData->m_GameplayDeviationScaleFactorZoom = 0.f;
	pSwayData->m_DeviationScaleFactorNoZoom = 0.f;
	pSwayData->m_GameplayDeviationScaleFactorNoZoom = 0.f;
	pSwayData->m_FirstShotRecoilMultiplier = 0.f;
	pSwayData->m_ShootingRecoilDecreaseScale = 100;

}
DWORD_PTR hkUpdate(DWORD_PTR a1, DWORD_PTR a2, DWORD_PTR a3)//ICY
{
	if (a1)
		*(bool*)(a1 + 0xEC) = true;

	if (a2)
		*(bool*)(a2 + 0x8A) = true;

	return oUpdate(a1, a2, a3);
}
__int32  __stdcall  hkPreFrame(float DeltaTime)
{




	__int32  returnval = oPreFrameUpdate(DeltaTime);

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
		Aimbot();
	}
	else {
		LockOnEme = false;
		LockOn_pEnemySoldier = nullptr;
	}

	for (int i = 0; i < 123; i++)
	{
		if (g_pInputBuffers[i] != 0.0f) { g_pBorderInputNode->m_InputCache->m_Event[i] = g_pInputBuffers[i]; }
		g_pInputBuffers[i] = 0.0f;
	}



	return returnval;
}
HRESULT __stdcall hkPresent(IDXGISwapChain* thisptr, UINT SyncInterval, UINT Flags) {


	SoldierWeaponUpgrade();






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

		*(intptr_t*)((intptr_t)pAntVtable + 11 * 8) = (intptr_t)hkUpdate;

		VirtualProtect((void*)((intptr_t)pAntVtable + 11 * 8), sizeof(void*), old, nullptr);





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
