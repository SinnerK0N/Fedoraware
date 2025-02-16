#include "AutoBlast.h"

#include "../AutoGlobal/AutoGlobal.h"
#include "../../Vars.h"

void CAutoAirblast::Run(CBaseEntity* pLocal, CBaseCombatWeapon* pWeapon, CUserCmd* pCmd)
{
	if (!Vars::Triggerbot::Blast::Active.m_Var || !g_GlobalInfo.m_bWeaponCanSecondaryAttack)
		return;

	id = pWeapon->GetWeaponID();

	if (id != TF_WEAPON_FLAMETHROWER && id != TF_WEAPON_FLAME_BALL)
		return;

	if (g_GlobalInfo.m_nCurItemDefIndex == Pyro_m_ThePhlogistinator)
		return;

	if (const auto& pNet = g_Interfaces.Engine->GetNetChannelInfo())
	{
		Vec3 vEyePos = pLocal->GetEyePosition();
		float flLatency = (pNet->GetLatency(FLOW_INCOMING) + pNet->GetLatency(FLOW_OUTGOING));
		bool bShouldBlast = false;

		for (const auto& pProjectile : g_EntityCache.GetGroup(EGroupType::WORLD_PROJECTILES))
		{
			if (pProjectile->GetTeamNum() == pLocal->GetTeamNum())
				continue; //Ignore team's projectiles

			switch (pProjectile->GetClassID())
			{
			case ETFClassID::CTFGrenadePipebombProjectile:
				{
					if (pProjectile->GetTouched())
						continue; //Ignore landed stickies

					break;
				}

			case ETFClassID::CTFProjectile_Arrow:
				{
					if (pProjectile->GetVecVelocity().IsZero())
						continue; //Ignore arrows with no velocity / not moving

					break;
				}

			default: break;
			}

			Vec3 vPredicted = (pProjectile->GetAbsOrigin() + pProjectile->GetVelocity().Scale(flLatency));

			//I cant remember if the airblast radius range from 2007 SDK was 185.0f or not..
			if (vEyePos.DistTo(vPredicted) <= 185.0f && Utils::VisPos(pLocal, pProjectile, vEyePos, vPredicted))
			{
				if (Vars::Triggerbot::Blast::Rage.m_Var)
				{
					pCmd->viewangles = Math::CalcAngle(vEyePos, vPredicted);
					bShouldBlast = true;
					break;
				}
				if (Math::GetFov(g_Interfaces.Engine->GetViewAngles(), vEyePos, vPredicted) <=
					Vars::Triggerbot::Blast::Fov.m_Var)
				{
					bShouldBlast = true;
					break;
				}
			}
		}

		if (bShouldBlast)
		{
			if (Vars::Triggerbot::Blast::Rage.m_Var && Vars::Triggerbot::Blast::Silent.m_Var)
				g_GlobalInfo.m_bSilentTime = true;

			g_GlobalInfo.m_bAttacking = true;
			pCmd->buttons |= IN_ATTACK2;
		}
	}
}
