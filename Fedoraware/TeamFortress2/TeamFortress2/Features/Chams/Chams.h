#pragma once
#include "../../SDK/SDK.h"

class CChams
{
private:
	IMaterial* m_pMatShaded, * m_pMatFlat, * m_pMatShiny, * m_pMatBrick, * m_pMatFresnelHDR0, * m_pMatFresnelHDR1, *m_pMatFresnel, * m_pMatPlastic;
	IMaterial* m_pMatBlur;

	IMaterialVar *m_pMatFresnelHDRSelfillumTint, *m_pMatFresnelHDREnvmapTint;

	std::map<CBaseEntity*, bool> m_DrawnEntities;

private:
	bool ShouldRun();
	void DrawModel(CBaseEntity* pEntity);

private:
	void RenderPlayers(CBaseEntity* pLocal, IMatRenderContext* pRenderContext);
	void RenderBuildings(CBaseEntity* pLocal, IMatRenderContext* pRenderContext);
	void RenderWorld(CBaseEntity* pLocal, IMatRenderContext* pRenderContext);
	IMaterial* fetchMaterial(Chams_t chams);

public:
	void Init();
	void Render();

	bool HasDrawn(CBaseEntity* pEntity)
	{
		return m_DrawnEntities.find(pEntity) != m_DrawnEntities.end();
	}

	bool IsChamsMaterial(IMaterial* pMat)
	{
		return (pMat == m_pMatShaded || pMat == m_pMatFlat ||
			pMat == m_pMatShiny || pMat == m_pMatBrick || pMat == m_pMatBlur
			|| pMat == m_pMatFresnel);
	}

public:
	bool m_bHasSetStencil;
	bool m_bRendering;
};

inline CChams g_Chams;
