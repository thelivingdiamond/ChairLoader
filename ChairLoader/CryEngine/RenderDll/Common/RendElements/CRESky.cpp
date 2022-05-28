// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include <CryRenderer/RenderElements/RendElement.h>
#include <CryRenderer/RenderElements/CRESky.h>
#include "Stars.h"
#include <Cry3DEngine/I3DEngine.h>

#include "../../XRenderD3D9/DriverD3D.h"

void CRESky::mfPrepare(bool bCheckOverflow)
{
	if (bCheckOverflow)
		gRenDev->FX_CheckOverflow(0, 0, this);

	gRenDev->m_RP.m_pRE = this;
	gRenDev->m_RP.m_RendNumIndices = 0;
	gRenDev->m_RP.m_RendNumVerts = 0;
}

EVertexFormat CRESky::GetVertexFormat() const
{
	return eVF_P3F_C4B_T2F;
}

bool CRESky::GetGeometryInfo(SGeometryInfo& streams, bool bSupportTessellation)
{
	ZeroStruct(streams);
	streams.eVertFormat = GetVertexFormat();
	streams.primitiveType = eptTriangleList;
	return true;
}

CRESky::~CRESky()
{
}

//////////////////////////////////////////////////////////////////////////
// HDR Sky
//////////////////////////////////////////////////////////////////////////

void CREHDRSky::GenerateSkyDomeTextures(int32 width, int32 height)
{
	SAFE_RELEASE(m_pSkyDomeTextureMie);
	SAFE_RELEASE(m_pSkyDomeTextureRayleigh);

	int creationFlags = FT_STATE_CLAMP | FT_NOMIPS;

	m_pSkyDomeTextureMie = CTexture::Create2DTexture("$SkyDomeTextureMie", width, height, 1, creationFlags, 0, eTF_R16G16B16A16F, eTF_R16G16B16A16F);
	m_pSkyDomeTextureMie->SetFilterMode(FILTER_LINEAR);
	m_pSkyDomeTextureMie->SetClampingMode(0, 1, 1);
	m_pSkyDomeTextureMie->UpdateTexStates();

	m_pSkyDomeTextureRayleigh = CTexture::Create2DTexture("$SkyDomeTextureRayleigh", width, height, 1, creationFlags, 0, eTF_R16G16B16A16F, eTF_R16G16B16A16F);
	m_pSkyDomeTextureRayleigh->SetFilterMode(FILTER_LINEAR);
	m_pSkyDomeTextureRayleigh->SetClampingMode(0, 1, 1);
	m_pSkyDomeTextureRayleigh->UpdateTexStates();

}

void CREHDRSky::Init()
{
	if (!m_pStars)
		m_pStars = new CStars;

	//No longer defer texture creation, MT resource creation now supported
	//gRenDev->m_pRT->RC_GenerateSkyDomeTextures(this, SSkyLightRenderParams::skyDomeTextureWidth, SSkyLightRenderParams::skyDomeTextureHeight);
	GenerateSkyDomeTextures(SSkyLightRenderParams::skyDomeTextureWidth, SSkyLightRenderParams::skyDomeTextureHeight);
}

void CREHDRSky::mfPrepare(bool bCheckOverflow)
{
	if (bCheckOverflow)
		gRenDev->FX_CheckOverflow(0, 0, this);

	//gRenDev->FX_CheckOverflow( 0, 0, this );
	gRenDev->m_RP.m_pRE = this;
	gRenDev->m_RP.m_RendNumIndices = 0;
	gRenDev->m_RP.m_RendNumVerts = 0;
}

EVertexFormat CREHDRSky::GetVertexFormat() const
{
	return eVF_P3F_C4B_T2F;
}

bool CREHDRSky::GetGeometryInfo(SGeometryInfo& streams, bool bSupportTessellation)
{
	ZeroStruct(streams);
	streams.eVertFormat = GetVertexFormat();
	streams.primitiveType = eptTriangleList;
	return true;
}

CREHDRSky::~CREHDRSky()
{
	SAFE_DELETE(m_pStars);

	SAFE_RELEASE(m_pSkyDomeTextureMie);
	SAFE_RELEASE(m_pSkyDomeTextureRayleigh);
}

void CREHDRSky::SetCommonMoonParams(CShader* ef, bool bUseMoon)
{

	I3DEngine* p3DEngine(gEnv->p3DEngine);

	Vec3 mr;
	p3DEngine->GetGlobalParameter(E3DPARAM_SKY_MOONROTATION, mr);
	float moonLati = -gf_PI + gf_PI * mr.x / 180.0f;
	float moonLong = 0.5f * gf_PI - gf_PI * mr.y / 180.0f;

	float sinLonR = sinf(-0.5f * gf_PI);
	float cosLonR = cosf(-0.5f * gf_PI);
	float sinLatR = sinf(moonLati + 0.5f * gf_PI);
	float cosLatR = cosf(moonLati + 0.5f * gf_PI);
	Vec3 moonTexGenRight(sinLonR * cosLatR, sinLonR * sinLatR, cosLonR);

	Vec4 nsMoonTexGenRight(moonTexGenRight, 0);
	static CCryNameR ParamNameTGR("SkyDome_NightMoonTexGenRight");
	ef->FXSetVSFloat(ParamNameTGR, &nsMoonTexGenRight, 1);

	float sinLonU = sinf(moonLong + 0.5f * gf_PI);
	float cosLonU = cosf(moonLong + 0.5f * gf_PI);
	float sinLatU = sinf(moonLati);
	float cosLatU = cosf(moonLati);
	Vec3 moonTexGenUp(sinLonU * cosLatU, sinLonU * sinLatU, cosLonU);

	Vec4 nsMoonTexGenUp(moonTexGenUp, 0);
	static CCryNameR ParamNameTGU("SkyDome_NightMoonTexGenUp");
	ef->FXSetVSFloat(ParamNameTGU, &nsMoonTexGenUp, 1);

	Vec3 nightMoonDirection;
	p3DEngine->GetGlobalParameter(E3DPARAM_NIGHSKY_MOON_DIRECTION, nightMoonDirection);
	float nightMoonSize(25.0f - 24.0f * clamp_tpl(p3DEngine->GetGlobalParameter(E3DPARAM_NIGHSKY_MOON_SIZE), 0.0f, 1.0f));
	Vec4 nsMoonDirSize(nightMoonDirection, bUseMoon ? nightMoonSize : 9999.0f);
	static CCryNameR ParamNameDirSize("SkyDome_NightMoonDirSize");
	ef->FXSetVSFloat(ParamNameDirSize, &nsMoonDirSize, 1);
	ef->FXSetPSFloat(ParamNameDirSize, &nsMoonDirSize, 1);
}

//////////////////////////////////////////////////////////////////////////
// Stars
//////////////////////////////////////////////////////////////////////////

CStars::CStars()
	: m_numStars(0)
	, m_pStarMesh(0)
	, m_pShader(0)
{
	if (LoadData())
	{

		gRenDev->m_cEF.mfRefreshSystemShader("Stars", gRenDev->m_cEF.s_ShaderStars);
		m_pShader = gRenDev->m_cEF.s_ShaderStars;

	}
}

CStars::~CStars()
{
	m_pStarMesh = NULL;
}

bool CStars::LoadData()
{
	const uint32 c_fileTag(0x52415453);       // "STAR"
	const uint32 c_fileVersion(0x00010001);
	const char c_fileName[] = "engineassets/sky/stars.dat";

	ICryPak* pPak(gEnv->pCryPak);
	if (pPak)
	{
		CInMemoryFileLoader file(pPak);
		if (file.FOpen(c_fileName, "rb"))
		{
			// read and validate header
			size_t itemsRead(0);
			uint32 fileTag(0);
			itemsRead = file.FRead(&fileTag, 1);
			if (itemsRead != 1 || fileTag != c_fileTag)
			{
				file.FClose();
				return false;
			}

			uint32 fileVersion(0);
			itemsRead = file.FRead(&fileVersion, 1);
			if (itemsRead != 1 || fileVersion != c_fileVersion)
			{
				file.FClose();
				return false;
			}

			// read in stars
			file.FRead(&m_numStars, 1);

			SVF_P3S_C4B_T2S* pData(new SVF_P3S_C4B_T2S[6 * m_numStars]);

			for (unsigned int i(0); i < m_numStars; ++i)
			{
				float ra(0);
				file.FRead(&ra, 1);

				float dec(0);
				file.FRead(&dec, 1);

				uint8 r(0);
				file.FRead(&r, 1);

				uint8 g(0);
				file.FRead(&g, 1);

				uint8 b(0);
				file.FRead(&b, 1);

				uint8 mag(0);
				file.FRead(&mag, 1);

				Vec3 v;
				v.x = -cosf(DEG2RAD(dec)) * sinf(DEG2RAD(ra * 15.0f));
				v.y = cosf(DEG2RAD(dec)) * cosf(DEG2RAD(ra * 15.0f));
				v.z = sinf(DEG2RAD(dec));

				for (int k = 0; k < 6; k++)
				{
					pData[6 * i + k].xyz = v;
					pData[6 * i + k].color.dcolor = (mag << 24) + (b << 16) + (g << 8) + r;
				}
			}

			m_pStarMesh = gRenDev->CreateRenderMeshInitialized(pData, 6 * m_numStars, eVF_P3S_C4B_T2S, 0, 0, prtTriangleList, "Stars", "Stars");

			delete[] pData;

			// check if we read entire file
			long curPos(file.FTell());
			file.FSeek(0, SEEK_END);
			long endPos(file.FTell());
			if (curPos != endPos)
			{
				file.FClose();
				return false;
			}

			file.FClose();
			return true;
		}
	}
	return false;
}
