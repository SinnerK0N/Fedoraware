#include "AntiAim.h"
#include "../Vars.h"

int edgeToEdgeOn = 0;
float lastRealAngle = -90.f;
float lastFakeAngle = 90.f;

void CAntiAim::FixMovement(CUserCmd* pCmd, const Vec3& vOldAngles, float fOldSideMove, float fOldForwardMove) {
	Vec3 curAngs = pCmd->viewangles;

	float fDelta = pCmd->viewangles.y - vOldAngles.y;
	float f1, f2;

	if (vOldAngles.y < 0.0f) { f1 = 360.0f + vOldAngles.y; }

	else { f1 = vOldAngles.y; }

	if (pCmd->viewangles.y < 0.0f) { f2 = 360.0f + pCmd->viewangles.y; }

	else { f2 = pCmd->viewangles.y; }

	if (f2 < f1) { fDelta = abs(f2 - f1); }

	else { fDelta = 360.0f - abs(f1 - f2); }

	fDelta = 360.0f - fDelta;

	pCmd->forwardmove = cos(DEG2RAD(fDelta)) * fOldForwardMove + cos(DEG2RAD(fDelta + 90.0f)) * fOldSideMove;
	pCmd->sidemove = sin(DEG2RAD(fDelta)) * fOldForwardMove + sin(DEG2RAD(fDelta + 90.0f)) * fOldSideMove;
}

float EdgeDistance(float edgeRayYaw) {
	// Main ray tracing area
	CGameTrace trace;
	Ray_t ray;
	Vector forward;
	const float sy = sinf(DEG2RAD(edgeRayYaw)); // yaw
	const float cy = cosf(DEG2RAD(edgeRayYaw));
	constexpr float sp = 0.f; // pitch: sinf(DEG2RAD(0))
	constexpr float cp = 1.f; // cosf(DEG2RAD(0))
	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
	forward = forward * 300.0f + g_EntityCache.m_pLocal->GetEyePosition();
	ray.Init(g_EntityCache.m_pLocal->GetEyePosition(), forward);
	// trace::g_pFilterNoPlayer to only focus on the enviroment
	CTraceFilterWorldAndPropsOnly Filter = {};
	g_Interfaces.EngineTrace->TraceRay(ray, 0x4200400B, &Filter, &trace);

	const float edgeDistance = (trace.vStartPos - trace.vEndPos).Length2D();
	return edgeDistance;
}

bool FindEdge(float edgeOrigYaw) {
	// distance two vectors and report their combined distances
	float edgeLeftDist = EdgeDistance(edgeOrigYaw - 21);
	edgeLeftDist = edgeLeftDist + EdgeDistance(edgeOrigYaw - 27);
	float edgeRightDist = EdgeDistance(edgeOrigYaw + 21);
	edgeRightDist = edgeRightDist + EdgeDistance(edgeOrigYaw + 27);

	// If the distance is too far, then set the distance to max so the angle
	// isnt used
	if (edgeLeftDist >= 260) { edgeLeftDist = 999999999.f; }
	if (edgeRightDist >= 260) { edgeRightDist = 999999999.f; }

	// If none of the vectors found a wall, then dont edge
	if (Utils::CompareFloat(edgeLeftDist, edgeRightDist)) { return false; }

	// Depending on the edge, choose a direction to face
	if (edgeRightDist < edgeLeftDist) {
		edgeToEdgeOn = 1;
		if (Vars::AntiHack::AntiAim::Pitch.m_Var == 1 ||
			Vars::AntiHack::AntiAim::Pitch.m_Var == 3 ||
			g_GlobalInfo.m_vRealViewAngles.x < 10.f) // Check for real up
		{
			edgeToEdgeOn = 2;
		}
		return true;
	}

	edgeToEdgeOn = 2;
	if (Vars::AntiHack::AntiAim::Pitch.m_Var == 1 ||
		Vars::AntiHack::AntiAim::Pitch.m_Var == 3 ||
		g_GlobalInfo.m_vRealViewAngles.x < 10.f) // Check for real up
	{
		edgeToEdgeOn = 1;
	}

	return true;
}

void CAntiAim::Run(CUserCmd* pCmd, bool* pSendPacket) {
	g_GlobalInfo.m_bAAActive = false;
	g_GlobalInfo.m_vRealViewAngles = g_GlobalInfo.m_vViewAngles;
	g_GlobalInfo.m_vFakeViewAngles = g_GlobalInfo.m_vViewAngles;

	if (!Vars::AntiHack::AntiAim::Active.m_Var || g_GlobalInfo.m_bForceSendPacket) { return; }

	if (const auto& pLocal = g_EntityCache.m_pLocal) {
		if (!pLocal->IsAlive()
			|| pLocal->IsTaunting()
			|| pLocal->IsInBumperKart()
			|| pLocal->IsAGhost()) { return; }

		if (g_GlobalInfo.m_bAttacking) { return; }
		if (const auto& pWeapon = g_EntityCache.m_pLocalWeapon) { if (Utils::IsAttacking(pCmd, pWeapon)) { return; } }

		static bool bSendReal = false;
		bool bPitchSet = true;
		bool bYawSet = true;

		const Vec3 vOldAngles = pCmd->viewangles;
		const float fOldSideMove = pCmd->sidemove;
		const float fOldForwardMove = pCmd->forwardmove;

		Vec3 vAngles = pCmd->viewangles;

		switch (Vars::AntiHack::AntiAim::Pitch.m_Var) {
		case 1:
			{
				pCmd->viewangles.x = -89.0f;
				g_GlobalInfo.m_vRealViewAngles.x = -89.0f;
				break;
			}
		case 2:
			{
				pCmd->viewangles.x = 89.0f;
				g_GlobalInfo.m_vRealViewAngles.x = 89.0f;
				break;
			}
		case 3:
			{
				pCmd->viewangles.x = -271.0f;
				g_GlobalInfo.m_vRealViewAngles.x = 89.0f;
				break;
			}
		case 4:
			{
				pCmd->viewangles.x = 271.0f;
				g_GlobalInfo.m_vRealViewAngles.x = -89.0f;
				break;
			}
		case 5:
			{
				pCmd->viewangles.x = Utils::RandFloatRange(-89.0f, 89.0f);
				g_GlobalInfo.m_vRealViewAngles.x = pCmd->viewangles.x; //Utils::RandFloatRange(-89.0f, 89.0f); this is bad
				break;
			}
		default:
			{
				bPitchSet = false;
				break;
			}
		}

		if (Vars::AntiHack::AntiAim::YawReal.m_Var == 6 || Vars::AntiHack::AntiAim::YawFake.m_Var == 6) {
			FindEdge(pCmd->viewangles.y);
		}

		if (bSendReal) {
			switch (Vars::AntiHack::AntiAim::YawReal.m_Var) {
			case 1:
				{
					pCmd->viewangles.y += 90.0f;
					break;
				}
			case 2:
				{
					pCmd->viewangles.y -= 90.0f;
					break;
				}
			case 3:
				{
					pCmd->viewangles.y += 180.0f;
					break;
				}
			case 4:
				{
					pCmd->viewangles.y = Utils::RandFloatRange(-180.0f, 180.0f);
					break;
				}
			case 5:
				{
					lastRealAngle += Vars::AntiHack::AntiAim::SpinSpeed.m_Var;
					if (lastRealAngle > 180.f) { lastRealAngle -= 360.f; }
					if (lastRealAngle < -180.f) { lastRealAngle += 360.f; }
					pCmd->viewangles.y = lastRealAngle;
					break;
				}
			case 6:
				{
					if (edgeToEdgeOn == 1) { pCmd->viewangles.y += 90; }
					else if (edgeToEdgeOn == 2) { pCmd->viewangles.y -= 90.0f; }
					break;
				}
			default:
				{
					bYawSet = false;
					break;
				}
			}

			g_GlobalInfo.m_vRealViewAngles.y = pCmd->viewangles.y;
		}

		else {
			switch (Vars::AntiHack::AntiAim::YawFake.m_Var) {
			case 1:
				{
					pCmd->viewangles.y += 90.0f;
					break;
				}
			case 2:
				{
					pCmd->viewangles.y -= 90.0f;
					break;
				}
			case 3:
				{
					pCmd->viewangles.y += 180.0f;
					break;
				}
			case 4:
				{
					pCmd->viewangles.y = Utils::RandFloatRange(-180.0f, 180.0f);
					break;
				}
			case 5:
				{
					lastFakeAngle += Vars::AntiHack::AntiAim::SpinSpeed.m_Var;
					if (lastFakeAngle > 180.f) { lastFakeAngle -= 360.f; }
					if (lastFakeAngle < -180.f) { lastFakeAngle += 360.f; }
					pCmd->viewangles.y = lastFakeAngle;
					break;
				}
			case 6:
				{
					if (edgeToEdgeOn == 1) { pCmd->viewangles.y -= 90; }
					else if (edgeToEdgeOn == 2) { pCmd->viewangles.y += 90.0f; }
					break;
				}
			default:
				{
					bYawSet = false;
					break;
				}
			}

			g_GlobalInfo.m_vFakeViewAngles = pCmd->viewangles;
		}

		if (bYawSet) { *pSendPacket = bSendReal = !bSendReal; }
		g_GlobalInfo.m_bAAActive = bPitchSet || bYawSet;

		FixMovement(pCmd, vOldAngles, fOldSideMove, fOldForwardMove);
	}
}
