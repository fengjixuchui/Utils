
#include "stdafx.h"

#include "Aimbot.h"
#include "poly34.h"

#include "hkheader.h"
double Aimbot::FindMinPos(double a[2]) {
	if (a[0] <= 0 && a[1] <= 0)return 0.0;

	else if (a[0] <= 0)return a[1];

	else if (a[1] <= 0)return a[0];

	else return a[0] < a[1] ? a[0] : a[1];
}
double Aimbot::TimeToHit(fb::Vec4 p, fb::Vec4 u, double v_pow2)
{
	double a = v_pow2 - u.Dot(u);
	double b = -2 * p.Dot(u);
	double c = -1 * p.Dot(p);


	double t0[2] = { 0 };


	double de = b*b - 4 * a*c;



	if (de >= 0)
	{
		t0[0] = (-b + sqrt(de)) / (2 * a);

		t0[1] = (-b - sqrt(de)) / (2 * a);
	}
	else
	{
		return -2.0f;
	}

	return FindMinPos(t0);

}



double Aimbot::FindMinPosRoot(double *a, int n) {

	if (n == 2) {

		return FindMinPos(a);

	}
	else {

		double tmp[2];

		tmp[0] = FindMinPos(a);

		tmp[1] = FindMinPos(&(a[2]));

		return FindMinPos(tmp);

	}


	return 0.0;

}
Aimbot::Aimbot()
{
	NullTmpVar();
	VecOfClosestSoldier = new fb::Vec4;
	this->bAimHead = false;
	this->LockOnEnemyFlags = 0;
	this->m_LockOn_pEnemySoldier = nullptr;
	this->m_LockOn_pEnemyPlayer = nullptr;
	this->mb_AimKeyPressed = false;
}

float Aimbot::DistanceToAimRay(fb::Vec4 MyPosition, fb::Vec4 EnemyPosition,
	const fb::Vec4 vAngle) {
	__try {
		float fYawDifference, flPitchDifference;

		fb::Vec4 vDir = EnemyPosition - MyPosition;
		float m_dist0 = vDir.len();
		float x, y;
		vDir.normalize();
		fYawDifference = -atan2(vDir.x, vDir.z);

		if (fYawDifference < 0)fYawDifference = fYawDifference + Twice_PI;


		fYawDifference = abs(fYawDifference - vAngle.x);

		if (m_dist0 <= 6.f && fYawDifference < 3.0f)return 100000000.f;



		if (fYawDifference > 0.7f)return -2.f;



		flPitchDifference = atan2(vDir.y, vDir.VectorLength2());

		flPitchDifference = abs(flPitchDifference - vAngle.y);

		if (flPitchDifference > 1.22f)return -2.f;

		x = cos(flPitchDifference)*sin(fYawDifference);

		y = sin(flPitchDifference);

		return x*x + y*y;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) { return -2.f; };
}

DWORD Aimbot::GetVectorFromVehicle(fb::ClientPlayer* pLocalPlayer, fb::Vec4* vector) {

	fb::LinearTransform* mTransform = new fb::LinearTransform;
	fb::Vec4  tmp;

	fb::ClientVehicleEntity *veh = pLocalPlayer->GetVehicleEntity();

	if (!IsValidPtr(veh))return 0x1;

	veh->GetTransform(mTransform);

	if (!IsValidPtr(mTransform))return 0x1;

	fb::AxisAlignedBox AABB = *(fb::AxisAlignedBox *) ((intptr_t)veh + 0x250);



	vector->x = mTransform->_41;
	vector->y = mTransform->_42;
	vector->z = mTransform->_43;
	vector->w = 0.f;

	tmp = (AABB.m_Max + AABB.m_Min)*0.5f;//model martix

	vector->x = vector->x + tmp.x*mTransform->_11 + tmp.y*mTransform->_21 + tmp.z*mTransform->_31;
	vector->y = vector->y + tmp.x*mTransform->_12 + tmp.y*mTransform->_22 + tmp.z*mTransform->_32;
	vector->z = vector->z + tmp.x*mTransform->_13 + tmp.y*mTransform->_23 + tmp.z*mTransform->_33;
	//vector->w = vector->w + tmp.x*mTransform->_14 + tmp.y*mTransform->_24 + tmp.z*mTransform->_34 + tmp.w*mTransform->_44;
	delete mTransform;
	return 0x0;
}

fb::Vec4  Aimbot::GetOriginAndUpdateCurrentAngle(fb::ClientPlayer* pLocalPlayer,
	fb::ClientWeapon* MyCSW, fb::FiringFunctionData* pFFD, bool b_InVeh) {
	fb::LinearTransform ShootSpaceMat;
	if (b_InVeh) {

		fb::MainVarPtr* turrent = fb::MainVarPtr::Singleton();

		ShootSpaceMat = turrent->m_turretTransform;


	}
	else {

		ShootSpaceMat = MyCSW->m_ShootSpace;

	}


	//typedef fb::Vec4* (__fastcall* Forward_vec)(fb::ClientWeapon* , fb::Vec4* );
	//
	//Forward_vec fn_calcu = (Forward_vec)(CALCU_VEC_FORWARD);

	fb::Vec4 vec_forward = ShootSpaceMat.forward;

	//	vec_forward = fn_calcu(MyCSW, &ShootSpaceMat.forward);

	this->v_curAngle.x = -atan2(vec_forward.x, vec_forward.z);

	//	this->v_curAngle.x = -atan2(ShootSpaceMat._31, ShootSpaceMat._33);



	if (this->v_curAngle.x < 0) { this->v_curAngle.x = this->v_curAngle.x + Twice_PI; }

	//this->v_curAngle.y = atan2(ShootSpaceMat._32, sqrt(ShootSpaceMat._33* ShootSpaceMat._33 + ShootSpaceMat._31* ShootSpaceMat._31));


	this->v_curAngle.y = atan2(vec_forward.y, sqrt(vec_forward.z* vec_forward.z + vec_forward.x* vec_forward.x));

	fb::Vec4 v_Tmp = ShootSpaceMat.trans;
	fb::Vec4 v_Up = ShootSpaceMat.up;
	fb::Vec4 v_Right = ShootSpaceMat.right;


	this->vOrigin = v_Tmp + v_Up*pFFD->m_ShotConfigData.m_InitialPosition.y + v_Right*pFFD->m_ShotConfigData.m_InitialPosition.x + vec_forward * pFFD->m_ShotConfigData.m_InitialPosition.z;

	this->vOrigin.w = 0;
	return this->vOrigin;


}




fb::Vec4 * Aimbot::GetClosestPlayer(eastl::vector<fb::ClientPlayer*>* pVecCP, fb::ClientPlayer* pLocalPlayer,
	bool missile, bool InVeh) {

	if (pVecCP->size() < 1) return nullptr;

	bool b_found = false;
	fb::Vec4 v_EnemyVecTmp;

	float flScreenDistance = -1.f;

	fb::ClientPlayer* pClientPlayer = nullptr;
	fb::ClientSoldierEntity* pEnemySoldier = nullptr;
	fb::ClientSoldierEntity* ClosestSold = nullptr;

	NullTmpVar();
	float closestdistance = 80000.f;

	if (!LockOnEnemyFlags) {


		for (int i = 0; i < pVecCP->size(); i++)
		{
			__try {
				pClientPlayer = pVecCP->at(i);


				//	if (!IsValidPtr(pClientPlayer))
				//		continue;

				if (pLocalPlayer->m_TeamId == pClientPlayer->m_TeamId)continue;


				pEnemySoldier = pClientPlayer->GetSoldierEntity();


				//	if (!IsValidPtr(pEnemySoldier))continue;

			//		if (!IsAlive(pEnemySoldier))continue;

				//	if (!pClientPlayer->InVehicle() && pEnemySoldier->m_Occluded)continue;

				fb::RagdollComponent* pRagdoll = *(fb::RagdollComponent**)((intptr_t)pEnemySoldier + 0x580);

				if (!IsValidPtr(pRagdoll))continue;

				if (bAimHead)
				{
					b_found = pRagdoll->GetBone(fb::UpdatePoseResultData::Head, &v_EnemyVecTmp);
				}

				else
				{
					b_found = pRagdoll->GetBone(fb::UpdatePoseResultData::Spine2, &v_EnemyVecTmp);
				}




				if (!b_found) {

					if ((pClientPlayer->InVehicle() && missile) || InVeh) {
						if (GetVectorFromVehicle(pClientPlayer, &v_EnemyVecTmp) != 0)continue;

					}

					else {
						continue;
					}

				}



				flScreenDistance = DistanceToAimRay(vOrigin, v_EnemyVecTmp, v_curAngle);


				if (flScreenDistance < 0)continue;




				if (flScreenDistance < closestdistance)
				{
					closestdistance = flScreenDistance;
					mp_ClosestSoldier = pEnemySoldier;
					mp_ClosestPlayer = pClientPlayer;
					*VecOfClosestSoldier = v_EnemyVecTmp;

				}
				else if (flScreenDistance > 100000.f) {
					mp_ClosestSoldier = pEnemySoldier;
					mp_ClosestPlayer = pClientPlayer;
					*VecOfClosestSoldier = v_EnemyVecTmp;
					break;

				}

			}
			__except (EXCEPTION_EXECUTE_HANDLER) {



				continue;
			}
		}


	}
	else {
		__try {
			pEnemySoldier = this->m_LockOn_pEnemySoldier;


			//	if (!IsAlive(pEnemySoldier))return nullptr;

			pClientPlayer = this->m_LockOn_pEnemyPlayer;


			fb::RagdollComponent* pRagdoll = *(fb::RagdollComponent**)((intptr_t)pEnemySoldier + 0x580);


			if (bAimHead)
			{
				b_found = pRagdoll->GetBone(fb::UpdatePoseResultData::Head, &v_EnemyVecTmp);



			}

			else
			{
				b_found = pRagdoll->GetBone(fb::UpdatePoseResultData::Spine2, &v_EnemyVecTmp);



			}




			if (!b_found) {

				if ((pClientPlayer->InVehicle() && missile) || InVeh) {


					if (GetVectorFromVehicle(pClientPlayer, &v_EnemyVecTmp) != 0)return nullptr;


				}




				else {
					return nullptr;
				}
			}



			mp_ClosestPlayer = pClientPlayer;
			mp_ClosestSoldier = pEnemySoldier;
			*VecOfClosestSoldier = v_EnemyVecTmp;
		}
		__except (EXCEPTION_EXECUTE_HANDLER) {


			return nullptr;
		}

	}

	if (mp_ClosestPlayer && mp_ClosestSoldier) {
		return 	VecOfClosestSoldier;
	}
	else { return nullptr; }

}

fb::Vec4 Aimbot::GetVecOfPlayer(bool bAimHead, fb::RagdollComponent* pRagdoll) {
	fb::Vec4 v_EnemyVecTmp;

	bool found = false;


}




void Aimbot::NullTmpVar() {
	mp_ClosestPlayer = nullptr;
	mp_ClosestSoldier = nullptr;

};
DWORD  Aimbot::AimCorrection2(fb::Vec4 MyPosition,
	fb::Vec4  EnemyP, fb::Vec4 EnemyVelocity, fb::Vec4  v1, float Gravity, fb::Vec4* out) {
	__try {

		float flPitch, flYaw, f_time, v0_pow2 = v1.z*v1.z + v1.y + v1.y; // x, tmp,

		fb::Vec4 v_Driection;

		EnemyP = EnemyP - MyPosition;
		MyPosition.w = 0;
		EnemyP.w = 0;
		EnemyVelocity.w = 0;



		fb::Vec4 GravityVec;



		if (Gravity != 0.f) {


			GravityVec.x = 0;

			GravityVec.y = Gravity;

			GravityVec.z = 0;
			GravityVec.w = 0;



			double res[4];

			double mmm = 0.25*Gravity*Gravity;

			double a = -1 * EnemyVelocity.Dot(GravityVec) / mmm;

			double b = (EnemyVelocity.Dot(EnemyVelocity) - EnemyP.Dot(GravityVec) - v0_pow2) / mmm;



			double c = (2.0f*(EnemyP.Dot(EnemyVelocity))) / mmm;

			double d = EnemyP.Dot(EnemyP) / mmm;

			int res1 = SolveP4(res, a, b, c, d);



			if (res1 == 0) { return 0x1; }

			else {
				//sprintf_s(buffer, 0xff, "root:%f,%f,%f,%f", res[0], res[1], res[2], res[3]); log(buffer);
				f_time = (float)FindMinPosRoot(res, res1);
				//sprintf_s(buffer, 0xff, "time:%f,", time); log(buffer);
			}




			if (f_time <= 0)return 0x1;



			v_Driection = EnemyP + EnemyVelocity*f_time;





			flPitch = asinf((v_Driection.y - 0.5f*Gravity*f_time*f_time) / (f_time*sqrt(v0_pow2)));


			//int i = 0;

			//flPitch = 0;
			//x = MyPosition.DistanceToVector(EnemyPosition);



			//for (; i <= 5; i++) {



			//	time = abs(x / (v0));

			//	EnemyPosition.x = EnemyP.x + EnemyVelocity.x*time;
			//	EnemyPosition.y = EnemyP.y + EnemyVelocity.y*time;
			//	EnemyPosition.z = EnemyP.z + EnemyVelocity.z*time;

			//	Driection = EnemyPosition - MyPosition;

			//	x = abs(Driection.VectorLength2());

			//	tmp = pow(v0, 4) - (Gravity*((Gravity*(x)*(x)) + 2 * (Driection.y)*v0*v0));

			//	if (tmp < 0)return 0x1;

			//	tmp = (v0*v0 - sqrt(tmp)) / (Gravity*x);

			//	if (_isnanf(tmp))return 0x1;

			//	flPitch = atanf(tmp);

			//	if (_isnanf(flPitch))return 0x1;

			//	if (EnemyVelocity.x == 0.f&& EnemyVelocity.y == 0.f&&EnemyVelocity.z == 0.f)break;

			//	if (x <= 5)break;



			//};

			//time = abs(x / (v0*cos(flPitch)));

			//EnemyPosition.x = EnemyP.x + EnemyVelocity.x*time;
			//EnemyPosition.y = EnemyP.y + EnemyVelocity.y*time;
			//EnemyPosition.z = EnemyP.z + EnemyVelocity.z*time;

			//Driection = EnemyPosition - MyPosition;



		}
		else {


			f_time = (float)TimeToHit(EnemyP, EnemyVelocity, v0_pow2);

			if (f_time < 0)return 0x1;



			v_Driection = EnemyP + EnemyVelocity*f_time;




			flPitch = atan2(v_Driection.y, v_Driection.VectorLength2());



		}


		if (isnormal(v1.y))	flPitch = flPitch - atan2(v1.y, v1.z);

		if (flPitch > 1.48350f || flPitch < -1.2217f || _isnanf(flPitch))return 0x2;



		flYaw = -atan2(v_Driection.x, v_Driection.z);  //y



		if (flYaw < 0)flYaw = flYaw + Twice_PI;


		if (flYaw < 0 || flYaw >Twice_PI || _isnanf(flYaw))return 0x3;



		out->x = flYaw;

		out->y = flPitch;
	}
	__except (EXCEPTION_EXECUTE_HANDLER)
	{

		return 0x4;
	}

	return 0x0;
}