// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"

#include "D3DStereo.h"
#include "DriverD3D.h"
#include "D3DPostProcess.h"
#include "D3DOculus.h"
#include "D3DOsvr.h"
#include "D3DOpenVR.h"

#define MGPU_STEREO

#if CRY_PLATFORM_WINDOWS
	#include <CryInput/IHardwareMouse.h>
#endif

#if defined(USE_NV_API)
	#include <NVAPI/nvapi.h>
#endif

#include <CrySystem/VR/IHMDManager.h>
#include <CrySystem/VR/IHMDDevice.h>
#ifdef ENABLE_BENCHMARK_SENSOR
	#include <IBenchmarkFramework.h>
	#include <IBenchmarkRendererSensorManager.h>
#endif

#if defined(AMD_LIQUID_VR) && !defined(OPENGL)
	#ifdef _M_AMD64
		#define DXX_DLL_NAME "atidxx64.dll"
		#define CFX_DLL_NAME "aticfx64.dll"
	#else
		#define DXX_DLL_NAME "atidxx32.dll"
		#define CFX_DLL_NAME "aticfx32.dll"
	#endif

IAmdDxExt* g_pDxxExt = NULL;
IAmdDxExtMgpuAppControl* g_pMgpu = NULL;
HANDLE g_drawDoneSync = NULL;
HANDLE g_xferDoneSync = NULL;
#elif defined(USE_NV_API)
void* g_pMgpu;
	#define SELECT_GPU(a)
	#define SELECT_ALL_GPU()
#else
void* g_pMgpu;
	#define SELECT_GPU(a)
	#define SELECT_ALL_GPU()
#endif

CD3DStereoRenderer::CD3DStereoRenderer(CD3D9Renderer& renderer, EStereoDevice device)
	: m_renderer(renderer)
	, m_device(device)
	, m_deviceState(STEREO_DEVSTATE_UNSUPPORTED_DEVICE)
	, m_mode(STEREO_MODE_NO_STEREO)
	, m_output(STEREO_OUTPUT_STANDARD)
	, m_submission(STEREO_SUBMISSION_SEQUENTIAL)
	, m_driver(DRIVER_UNKNOWN)
	, m_nvStereoHandle(nullptr)
	, m_nvStereoStrength(0.0f)
	, m_nvStereoActivated(0)
	, m_curEye(LEFT_EYE)
	, m_frontBufWidth(0)
	, m_frontBufHeight(0)
	, m_stereoStrength(0.0f)
	, m_zeroParallaxPlaneDist(0.25f)
	, m_maxSeparationScene(0.0f)
	, m_nearGeoScale(0.0f)
	, m_gammaAdjustment(0)
	, m_screenSize(0.0f)
	, m_systemCursorNeedsUpdate(false)
	, m_resourcesPatched(false)
	, m_needClearLeft(true)
	, m_needClearRight(true)
	, m_needClearVrQuadLayer(true)
	, m_pHmdRenderer(nullptr)
	, m_pHmdDevice(nullptr)
	, m_bAsyncCameraMatrixValid(false)
	, m_asyncCameraMatrix(IDENTITY)
{
	if (device == STEREO_DEVICE_DEFAULT)
		SelectDefaultDevice();

	memset(m_pVrQuadLayerTex, 0, sizeof(m_pVrQuadLayerTex));
	m_bDisplayStereoDone = false;

#if defined(AMD_LIQUID_VR) && !defined(OPENGL)
	//////////////////////////////////////////////////////////////////////////
	// INTIALIZE LIQUID VR (to be relocated where appropriate later)
	//////////////////////////////////////////////////////////////////////////
	static bool _hack_bUseMGPUAffinity = true;
	if (_hack_bUseMGPUAffinity)
	{
		HMODULE hCfx = LoadLibraryA(CFX_DLL_NAME);

		PFNAmdExtRequestMgpuAppControl pFnEnable =
		  (PFNAmdExtRequestMgpuAppControl)GetProcAddress(hCfx, "AmdExtRequestMgpuAppControl");

		if (pFnEnable != nullptr)
		{
			pFnEnable(TRUE);
			CryLogAlways("AMD LIQUID VR library loaded");
		}
		else
		{
			CryLogAlways("AMD LIQUID VR library failed to loaded");
		}
	}
#endif

	m_pSideTexs[LEFT_EYE] = CTexture::s_ptexStereoL;
	m_pSideTexs[RIGHT_EYE] = CTexture::s_ptexStereoR;
}

CD3DStereoRenderer::~CD3DStereoRenderer()
{
	Shutdown();
}

void CD3DStereoRenderer::SelectDefaultDevice()
{
	EStereoDevice device = STEREO_DEVICE_NONE;

#if CRY_PLATFORM_WINDOWS
	device = STEREO_DEVICE_FRAMECOMP;
#elif CRY_PLATFORM_DURANGO || CRY_PLATFORM_ORBIS
	device = STEREO_DEVICE_FRAMECOMP;
#elif CRY_PLATFORM_LINUX || CRY_PLATFORM_ANDROID || CRY_PLATFORM_APPLE
	device = STEREO_DEVICE_FRAMECOMP;
#endif

	m_device = device;
}

void CD3DStereoRenderer::InitDeviceBeforeD3D()
{
	LOADING_TIME_PROFILE_SECTION;
#if defined(USE_NV_API)
	bool availableDriverNV = false;
	uint8 enabled = 0;

	// Always set direct mode so that the NV driver never does its own stereo
	if (NvAPI_Initialize() == NVAPI_OK &&
	    NvAPI_Stereo_SetDriverMode(NVAPI_STEREO_DRIVER_MODE_DIRECT) == NVAPI_OK &&
	    NvAPI_Stereo_IsEnabled(&enabled) == NVAPI_OK)
	{
		if (!enabled)
			iLog->LogWarning("3DVision is disabled in control panel");
		else
			availableDriverNV = true;
	}

	if (!availableDriverNV && m_device == STEREO_DEVICE_DRIVER && CRenderer::CV_r_StereoDevice == STEREO_DEVICE_DEFAULT)
	{
		// Fall back to frame-compatible device
		m_device = STEREO_DEVICE_FRAMECOMP;
	}
#endif

	if (m_device == STEREO_DEVICE_NONE)
		return;

	bool success = true;
	m_deviceState = STEREO_DEVSTATE_UNSUPPORTED_DEVICE;

#if defined(USE_NV_API)
	if (m_device == STEREO_DEVICE_DRIVER)
	{
		if (availableDriverNV)
		{
			m_driver = DRIVER_NV;
		}
		else
		{
			success = false;
			m_deviceState = STEREO_DEVSTATE_BAD_DRIVER;
		}
	}
#elif CRY_PLATFORM_DURANGO || CRY_PLATFORM_ORBIS
	if (m_device != STEREO_DEVICE_FRAMECOMP)
		success = false;
#endif

	if (success)
	{
		m_deviceState = STEREO_DEVSTATE_OK;
	}
	else
	{
		iLog->LogError("Failed to create stereo device");
		m_device = STEREO_DEVICE_NONE;
	}
}

void CD3DStereoRenderer::InitDeviceAfterD3D()
{
	LOADING_TIME_PROFILE_SECTION;

	g_pMgpu = NULL;
#if defined(AMD_LIQUID_VR) && !defined(OPENGL)
	//TD need to add AMD lib check
	// LIQUID
	HMODULE hDxx = LoadLibraryA(DXX_DLL_NAME);

	if (hDxx != NULL)
	{
		PFNAmdDxExtCreate11 pFnCreate = (PFNAmdDxExtCreate11)GetProcAddress(hDxx, "AmdDxExtCreate11");

		if (pFnCreate != nullptr)
		{
			D3DDevice* device = NULL;
	#if defined(DIRECT3D10)
			device = m_renderer.GetDevice().GetRealDevice();
	#endif

			assert(device);
			pFnCreate(device, &g_pDxxExt);

			if (g_pDxxExt != nullptr)
			{
				IAmdDxExtInterface* pIface = g_pDxxExt->GetExtInterface(AmdDxExtMgpuAppControlID);

				if (pIface != nullptr)
				{
					g_pMgpu = static_cast<IAmdDxExtMgpuAppControl*>(pIface);
					if (g_pMgpu)
					{
						// Set our default masks
						g_pMgpu->SetRenderGpuMask(GPUMASK_BOTH);

						g_drawDoneSync = g_pMgpu->SyncCreate();
						g_xferDoneSync = g_pMgpu->SyncCreate();
					}
				}
			}
		}
	}
#endif

#if defined(USE_NV_API)
	if (IsDriver(DRIVER_NV))
	{
		if (m_nvStereoHandle)
		{
			NvAPI_Stereo_DestroyHandle(m_nvStereoHandle);
			m_nvStereoHandle = nullptr;
		}

		void* pStereoHandle;
		D3DDevice* device = m_renderer.GetDevice().GetRealDevice();

		if (NvAPI_Stereo_CreateHandleFromIUnknown(device, &pStereoHandle) == NVAPI_OK)
			m_nvStereoHandle = pStereoHandle;
		else
			m_device = STEREO_DEVICE_NONE;

		DisableStereo();  // Always disabled at startup
	}

	g_pMgpu = NULL;
#endif

	if (m_renderer.m_DeviceWrapper.GetNodeCount() & 2)
	{
		m_submission = STEREO_SUBMISSION_PARALLEL;

		if (m_renderer.m_DeviceWrapper.GetNodeCount() & 4)
		{
			m_submission = STEREO_SUBMISSION_PARALLEL_MULTIRES;
		}
	}
}

void CD3DStereoRenderer::CreateResources()
{
	m_SourceSizeParamName = CCryNameR("SourceSize");

	if (!CTexture::s_ptexStereoL || !CTexture::s_ptexStereoR)
	{
		CreateIntermediateBuffers();
	}
}

void CD3DStereoRenderer::CreateIntermediateBuffers()
{
	SAFE_RELEASE(CTexture::s_ptexStereoL);
	SAFE_RELEASE(CTexture::s_ptexStereoR);

	int nWidth = m_renderer.GetWidth();
	int nHeight = m_renderer.GetHeight();

	uint32 nFlags = FT_DONT_STREAM | FT_USAGE_RENDERTARGET;

	if (m_submission > STEREO_SUBMISSION_SEQUENTIAL)
	{
		// Allocate the following buffers with visibility to all nodes (to be able to copy them around)
		m_renderer->GetDevice_Unsynchronized().SwitchNodeVisibility(~UINT(0));
	}

	CTexture::s_ptexStereoL = CTexture::CreateRenderTarget("$StereoL", nWidth, nHeight, Clr_Empty, eTT_2D, nFlags, eTF_R8G8B8A8);
	CTexture::s_ptexStereoR = CTexture::CreateRenderTarget("$StereoR", nWidth, nHeight, Clr_Empty, eTT_2D, nFlags, eTF_R8G8B8A8);

	if (m_submission > STEREO_SUBMISSION_SEQUENTIAL)
	{
		m_renderer->GetDevice_Unsynchronized().SwitchNodeVisibility(0UL);
	}

	m_pSideTexs[LEFT_EYE] = CTexture::s_ptexStereoL;
	m_pSideTexs[RIGHT_EYE] = CTexture::s_ptexStereoR;
}

void CD3DStereoRenderer::Shutdown()
{
	ReleaseResources();

	OnHmdDeviceChanged(nullptr);

#if defined(USE_NV_API)
	if (m_nvStereoHandle)
	{
		NvAPI_Stereo_DestroyHandle(m_nvStereoHandle);
		m_nvStereoHandle = nullptr;
	}
#endif
}

void CD3DStereoRenderer::ReleaseResources()
{
	if (m_device == STEREO_DEVICE_NONE)
		return;

	SAFE_RELEASE(CTexture::s_ptexStereoL);
	SAFE_RELEASE(CTexture::s_ptexStereoR);
}

bool CD3DStereoRenderer::EnableStereo()
{
#if defined(USE_NV_API)
	m_systemCursorNeedsUpdate = true;

	if (IsDriver(DRIVER_NV))
	{
		uint8 windowedSupport = 0;
		NvAPI_Stereo_IsWindowedModeSupported(&windowedSupport);

		if (m_renderer.m_bFullScreen || windowedSupport)
		{
			NvAPI_Stereo_Activate(m_nvStereoHandle);
			NvAPI_Stereo_SetNotificationMessage(m_nvStereoHandle, 0, 0);
			NvAPI_Stereo_SetActiveEye(m_nvStereoHandle, NVAPI_STEREO_EYE_LEFT);
			return true;
		}
		else
		{
			NvAPI_Stereo_Deactivate(m_nvStereoHandle);
			iLog->LogError("3DVision requires fullscreen mode");
			return false;
		}
	}
#endif

	if (!InitializeHmdRenderer())
		return false;

	return true;
}

void CD3DStereoRenderer::DisableStereo()
{
#if defined(USE_NV_API)
	m_systemCursorNeedsUpdate = true;

	if (IsDriver(DRIVER_NV))
	{
		NvAPI_Stereo_Deactivate(m_nvStereoHandle);
	}
#endif

	m_gammaAdjustment = 0;

	ShutdownHmdRenderer();
}

void CD3DStereoRenderer::ChangeOutputFormat()
{
	m_frontBufWidth = 0;
	m_frontBufHeight = 0;

	ShutdownHmdRenderer();
	InitializeHmdRenderer();
}

bool CD3DStereoRenderer::InitializeHmdRenderer()
{
	assert(m_pHmdRenderer == nullptr);
	m_pHmdRenderer = CreateHmdRenderer(m_output, m_pHmdDevice, &m_renderer, this);

	if (m_pHmdRenderer != nullptr)
	{
		if (!m_pHmdRenderer->Initialize())
		{
			SAFE_DELETE(m_pHmdRenderer);
			return false;
		}

		gEnv->pLog->Log("[HMD] HMD Renderer initialized");
	}
	else
	{
		gEnv->pLog->Log("[HMD] HMD Renderer not initialized - stereoOuput [%d] hmdDevice [%s null]", m_output, m_pHmdDevice ? "NOT" : "");
	}

	return m_pHmdRenderer != nullptr;
}

void CD3DStereoRenderer::ShutdownHmdRenderer()
{
	if (m_pHmdRenderer != nullptr)
	{
		m_pHmdRenderer->Shutdown();
		SAFE_DELETE(m_pHmdRenderer);
	}
}

void CD3DStereoRenderer::PrepareStereo(EStereoMode mode, EStereoOutput output)
{
#if defined(USE_NV_API)
	if (m_systemCursorNeedsUpdate && gEnv->pHardwareMouse)
	{
		if (!gEnv->IsEditor() && !IsDriver(DRIVER_NV))
		{
			gEnv->pHardwareMouse->UseSystemCursor(IsStereoEnabled() == false);
		}
		m_systemCursorNeedsUpdate = false;
	}

	if (IsDriver(DRIVER_NV))
	{
		HandleNVControl();
	}
#endif

	if (m_mode != mode || m_output != output)
	{
		m_renderer.ForceFlushRTCommands();

		if (m_mode != mode)
		{
			m_mode = mode;
			m_output = output;

			if (mode == STEREO_MODE_NO_STEREO)
			{
				DisableStereo();
			}
			else
			{
				EnableStereo();
				ChangeOutputFormat();
			}
		}
		else if (m_output != output)
		{
			m_output = output;

			if (IsStereoEnabled())
				ChangeOutputFormat();
		}
	}

	if (IsStereoEnabled())
	{
		// VR FIX ME
		// These code was written for 3D TV stereo, not for VR.
		// VR EyeDist concept is the half distance between the eyes, it can be different for 3D TV with reprojection

		// Update stereo parameters
		m_stereoStrength = CRenderer::CV_r_StereoStrength;
		m_maxSeparationScene = CRenderer::CV_r_StereoEyeDist;
		m_zeroParallaxPlaneDist = CRenderer::CV_r_StereoScreenDist;
		m_nearGeoScale = CRenderer::CV_r_StereoNearGeoScale;
		m_gammaAdjustment = CRenderer::CV_r_StereoGammaAdjustment;

		float screenDiagonal = GetScreenDiagonalInInches();
		if (screenDiagonal > 0.0f) // override CVar if we can determine the correct maximum separation
		{
			float aspect = 9.0f / 16.0f;
			float horizontal = screenDiagonal / sqrtf(1.0f + aspect * aspect);
			float typicalEyeSeperation = 2.5f;                                                   // In inches
			m_maxSeparationScene = min(typicalEyeSeperation / horizontal, m_maxSeparationScene); // Get bleeding at edges so limit to CVar
		}

		// Apply stereo strength
		m_maxSeparationScene *= m_stereoStrength;

		if (m_pHmdRenderer != nullptr)
			m_pHmdRenderer->PrepareFrame();
	}
}

void CD3DStereoRenderer::HandleNVControl()
{
#if defined(USE_NV_API)
	if (m_stereoStrength != CRenderer::CV_r_StereoStrength)
	{
		NvAPI_Stereo_SetSeparation(m_nvStereoHandle, clamp_tpl((CRenderer::CV_r_StereoStrength - 0.5f) * 100.f, 0.f, 100.f));
	}

	NvAPI_Stereo_IsActivated(m_nvStereoHandle, &m_nvStereoActivated);
	NvAPI_Stereo_GetSeparation(m_nvStereoHandle, &m_nvStereoStrength);
#endif
}

void CD3DStereoRenderer::SetEyeTextures(CTexture* leftTex, CTexture* rightTex)
{
	m_pSideTexs[LEFT_EYE] = leftTex ? leftTex : CTexture::s_ptexStereoL;
	m_pSideTexs[RIGHT_EYE] = rightTex ? rightTex : CTexture::s_ptexStereoR;
}

void CD3DStereoRenderer::SetVrQuadLayerTexture(RenderLayer::EQuadLayers id, CTexture* pTexture)
{
	m_pVrQuadLayerTex[id] = pTexture;
}

void CD3DStereoRenderer::Update()
{
	if (m_device != STEREO_DEVICE_NONE)
	{
		m_renderer.m_pRT->RC_PrepareStereo(CRenderer::CV_r_StereoMode, CRenderer::CV_r_StereoOutput);
	}
	else
	{
		static int prevMode = 0;
		if (CRenderer::CV_r_StereoMode != prevMode)
		{
			LogWarning("No stereo device enabled, ignoring stereo mode");
			prevMode = CRenderer::CV_r_StereoMode;
		}
	}
}

CCamera CD3DStereoRenderer::PrepareCamera(int nEye, const CCamera& currentCamera)
{
	CCamera cam = currentCamera;

	bool renderToHmd = false;

	if (gEnv->pConsole)
	{
		if (ICVar* pCvar = gEnv->pConsole->GetCVar("sys_vr_support"))
		{
			renderToHmd = pCvar->GetIVal() > 0;
		}
	}

	if (gEnv->IsEditor() && !gEnv->IsEditorGameMode())
		renderToHmd = false;

	if (renderToHmd)
	{
		if (IHmdManager* pHmdManager = gEnv->pSystem->GetHmdManager())
		{
			IHmdManager::SAsymmetricCameraSetupInfo asymCamSetupInfo;
			if (pHmdManager->GetAsymmetricCameraSetupInfo(nEye, asymCamSetupInfo))
			{
				// Note:
				// The inter-pupilar distance (IPD) gets modulated by r_stereoScaleCoefficient in order to be able
				// to "scale" the feeling of the player's size.
				// It also modifies the movement of the player's pose tracking

				const float modulatedHalfIpd = asymCamSetupInfo.eyeDist * 0.5f * CRenderer::CV_r_stereoScaleCoefficient;
				const float transfAsymH = asymCamSetupInfo.asymH * cam.GetNearPlane();
				const float transfAsymV = asymCamSetupInfo.asymV * cam.GetNearPlane();

				Matrix34 stereoMat = Matrix34::CreateTranslationMat(Vec3(nEye == LEFT_EYE ? -modulatedHalfIpd : modulatedHalfIpd, 0, 0));
				cam.SetMatrix(cam.GetMatrix() * stereoMat);
				cam.SetFrustum(1, 1, asymCamSetupInfo.fov, cam.GetNearPlane(), cam.GetFarPlane(), 1.0f / asymCamSetupInfo.aspectRatio);
				cam.SetAsymmetry(transfAsymH, transfAsymH, transfAsymV, transfAsymV);
			}
		}
	}
	else
	{
		float fNear = cam.GetNearPlane();
		float screenDist = CRenderer::CV_r_StereoScreenDist;
		float z = 99999.0f;  // Point which is far away

		// standard 3D TV stereo projection parameters
		float wT = tanf(cam.GetFov() * 0.5f) * fNear;
		float wR = wT * cam.GetProjRatio(), wL = -wR;
		float p00 = 2 * fNear / (wR - wL);
		float p02 = (wL + wR) / (wL - wR);

		// Compute required camera shift, so that a distant point gets the desired maximum separation
		const float maxSeparation = CRenderer::CV_r_StereoEyeDist;
		const float camOffset = (maxSeparation - p02) / (p00 / z - p00 / screenDist);
		float frustumShift = camOffset * (fNear / screenDist);
		//frustumShift *= (nEye == LEFT_EYE ? 1 : -1);  // Support postive and negative parallax for non-VR stereo

		Matrix34 stereoMat = Matrix34::CreateTranslationMat(Vec3(nEye == LEFT_EYE ? -camOffset : camOffset, 0, 0));
		cam.SetMatrix(cam.GetMatrix() * stereoMat);
		cam.SetAsymmetry(frustumShift, frustumShift, 0, 0);
	}

	return cam;
}

void CD3DStereoRenderer::ProcessScene(int sceneFlags, const SRenderingPassInfo& passInfo)
{
	int nThreadID = passInfo.ThreadID();

	if (!RequiresSequentialSubmission() && CRenderer::CV_r_StereoMode == STEREO_MODE_DUAL_RENDERING && !passInfo.IsRecursivePass())
	{
		CCamera cam = m_renderer.m_RP.m_TI[nThreadID].m_cam;

		int sceneFlagsLeft = SHDF_NO_SHADOWGEN;
		int sceneFlagsRight = SHDF_NO_SHADOWGEN;

		// Left eye
		{
			if (g_pMgpu && CRenderer::CV_r_StereoEnableMgpu)
			{
				m_renderer->SelectGPU(GPUMASK_LEFT);
			}

			m_renderer.m_pRT->RC_SetStereoEye(LEFT_EYE);
			m_renderer.PushProfileMarker("LEFT_EYE");

			//			CCamera camL = PrepareCamera(LEFT_EYE, cam, passInfo);
			//			m_renderer.SetCamera(camL);

			RenderScene(sceneFlags | sceneFlagsLeft, passInfo);
			CopyToStereoFromMainThread(LEFT_EYE);

			m_renderer.PopProfileMarker("LEFT_EYE");
		}

		if (g_pMgpu && CRenderer::CV_r_StereoEnableMgpu)
		{
			m_renderer->SelectGPU(GPUMASK_BOTH);
		}
	}
	else if (CRenderer::CV_r_StereoMode == STEREO_MODE_DUAL_RENDERING && !passInfo.IsRecursivePass())   //for recursive rendering (e.g. rendering to ocean reflection texture), stereo is not needed
	{
		CCamera cam = m_renderer.m_RP.m_TI[nThreadID].m_cam;

		int sceneFlagsLeft = SHDF_STEREO_LEFT_EYE;
		int sceneFlagsRight = SHDF_NO_SHADOWGEN | SHDF_STEREO_RIGHT_EYE;

		if (g_pMgpu && CRenderer::CV_r_StereoEnableMgpu)
		{
			// Render shadows on both GPUs
			sceneFlagsLeft = 0;
			sceneFlagsRight = 0;

#if !defined(_RELEASE)
			// Divide the cascades across the GPUs
			CRenderer::CV_r_ShadowGenMode = 1;
#endif
		}

		// Left eye
		{
			if (g_pMgpu && CRenderer::CV_r_StereoEnableMgpu)
			{
				m_renderer->SelectGPU(GPUMASK_LEFT);
			}

			m_renderer.m_pRT->RC_SetStereoEye(LEFT_EYE);
			m_renderer.PushProfileMarker("LEFT_EYE");

			CCamera camL = PrepareCamera(LEFT_EYE, cam);
			m_renderer.SetCamera(camL);

			RenderScene(sceneFlags | sceneFlagsLeft, passInfo);
			CopyToStereoFromMainThread(LEFT_EYE);

			m_renderer.PopProfileMarker("LEFT_EYE");
		}

		// Right eye
		{
			if (g_pMgpu && CRenderer::CV_r_StereoEnableMgpu)
			{
				m_renderer->SelectGPU(GPUMASK_RIGHT);
			}

			m_renderer.m_pRT->RC_SetStereoEye(RIGHT_EYE);
			m_renderer.PushProfileMarker("RIGHT_EYE");

			CCamera camR = PrepareCamera(RIGHT_EYE, cam);
			m_renderer.SetCamera(camR);

			RenderScene(sceneFlags | sceneFlagsRight, passInfo);
			CopyToStereoFromMainThread(RIGHT_EYE);
			m_renderer.PopProfileMarker("RIGHT_EYE");
		}

		if (g_pMgpu && CRenderer::CV_r_StereoEnableMgpu)
		{
			m_renderer->SelectGPU(GPUMASK_BOTH);
		}
	}
	else
	{
		if (CRenderer::CV_r_StereoMode != STEREO_MODE_DUAL_RENDERING)
		{
			m_renderer.m_pRT->RC_SetStereoEye(0);
		}

		RenderScene(sceneFlags, passInfo);
	}
}

void CD3DStereoRenderer::CopyToStereo(int channel)
{
	assert(IsRenderThread());

	PROFILE_LABEL_SCOPE("COPY_TO_STEREO");

	CTexture* pTex;

	if (channel == 0)
	{
		//TD FIX with proper version
		//pTex = CTexture::s_ptexStereoL2;
		pTex = GetEyeTarget(LEFT_EYE);
		m_needClearLeft = false;
	}
	else
	{
		//TD FIX with proper version
		//pTex = CTexture::s_ptexStereoR2;

		pTex = GetEyeTarget(RIGHT_EYE);
		m_needClearRight = false;
	}

	if (pTex == nullptr)
		return;

	//TD FIX with proper version
	//GetUtils().CopyScreenToTexture( CTexture::s_ptexSceneDiffuse );
	//GetUtils().StretchRect(CTexture::s_ptexSceneDiffuse, pTex );

	GetUtils().CopyScreenToTexture(pTex);
}

void CD3DStereoRenderer::SubmitFrameToHMD()
{
	assert(IsRenderThread());

	if (!IsStereoEnabled() || m_renderer.m_bDeviceLost)  // When unloading level, m_bDeviceLost is set to 2
		return;

	if (m_pHmdRenderer != nullptr)
	{
		m_pHmdRenderer->SubmitFrame();
		m_pHmdRenderer->RenderSocialScreen();

		m_needClearVrQuadLayer = true;
	}
}

void CD3DStereoRenderer::DisplayStereo()
{
	assert(IsRenderThread());

	m_bDisplayStereoDone = true;

	if (!IsStereoEnabled() || m_renderer.m_bDeviceLost)  // When unloading level, m_bDeviceLost is set to 2
		return;

	if (m_pHmdRenderer != nullptr)
	{
		return;
	}

#ifdef ENABLE_BENCHMARK_SENSOR
	gcpRendD3D->m_benchmarkRendererSensor->PreStereoFrameSubmit(GetEyeTarget(LEFT_EYE), GetEyeTarget(RIGHT_EYE));
	gcpRendD3D->m_benchmarkRendererSensor->AfterStereoFrameSubmit();
#endif

	m_needClearLeft = true;
	m_needClearRight = true;

	if ((m_device == STEREO_DEVICE_FRAMECOMP) && (m_output == STEREO_OUTPUT_SIDE_BY_SIDE))
	{
		D3DResource* pTarget = nullptr;
		m_renderer->m_pNewTarget[0]->m_pTarget->GetResource(&pTarget);

		//	m_renderer.FX_ClearTarget(GetEyeTarget(LEFT_EYE), Clr_Median);
		//	m_renderer.FX_ClearTarget(GetEyeTarget(RIGHT_EYE), Clr_Median);

		m_renderer->GetDeviceContext_Unsynchronized().JoinSubresourceRegion(
		  pTarget,
		  0,
		  0,
		  0,
		  0,
		  GetEyeTarget(LEFT_EYE)->GetDevTexture()->GetBaseTexture(),
		  GetEyeTarget(RIGHT_EYE)->GetDevTexture()->GetBaseTexture(),
		  0,
		  nullptr);

		return;
	}

	// The following functions need 3DEngine
	if (!gEnv->p3DEngine)
		return;

	m_renderer.m_cEF.mfRefreshSystemShader("Stereo", CShaderMan::s_ShaderStereo);

	PROFILE_LABEL_SCOPE("DISPLAY_STEREO");

	//TD legacy old multi-gpu support
	//if (g_pMgpu && CRenderer::CV_r_StereoEnableMgpu)
	//{
	//	//D3D11_RECT fullRect = { m_renderer->GetWidth()*0.5, 0, m_renderer->GetWidth(), m_renderer->GetHeight() };
	//	D3D11_RECT fullRect = {0, 0, m_renderer->GetWidth() / 2, m_renderer->GetHeight() };

	//	g_pMgpu->TransferResource(
	//		GetEyeTarget(LEFT_EYE)->GetDevTexture()->GetBaseTexture(),
	//		GetEyeTarget(LEFT_EYE)->GetDevTexture()->GetBaseTexture(),
	//		1,
	//		0,
	//		0,
	//		0,
	//		&fullRect);
	//}

	// TODO: Fix this properly
	if (!gEnv->IsEditor())
		m_renderer.RT_SetViewport(0, 0, m_renderer.GetBackbufferWidth(), m_renderer.GetBackbufferHeight());

	CShader* pSH = m_renderer.m_cEF.s_ShaderStereo;
	SelectShaderTechnique();

	uint32 nPasses = 0;
	pSH->FXBegin(&nPasses, FEF_DONTSETSTATES);
	pSH->FXBeginPass(0);

	m_renderer.FX_SetActiveRenderTargets();
	m_renderer.FX_SetState(GS_NODEPTHTEST);

	int width = m_renderer.GetWidth();
	int height = m_renderer.GetHeight();

	Vec4 pParams = Vec4((float) width, (float) height, 0, 0);
	CShaderMan::s_shPostEffects->FXSetPSFloat(m_SourceSizeParamName, &pParams, 1);

	GetUtils().SetTexture(!CRenderer::CV_r_StereoFlipEyes ? GetEyeTarget(LEFT_EYE) : GetEyeTarget(RIGHT_EYE), 0, FILTER_LINEAR);
	GetUtils().SetTexture(!CRenderer::CV_r_StereoFlipEyes ? GetEyeTarget(RIGHT_EYE) : GetEyeTarget(LEFT_EYE), 1, FILTER_LINEAR);

	GetUtils().DrawFullScreenTri(width, height);

#if defined(USE_NV_API)
	if (IsDriver(DRIVER_NV) && m_nvStereoActivated)
	{
		NvAPI_Stereo_SetActiveEye(m_nvStereoHandle, NVAPI_STEREO_EYE_RIGHT);
		GetUtils().SetTexture(!CRenderer::CV_r_StereoFlipEyes ? GetEyeTarget(RIGHT_EYE) : GetEyeTarget(LEFT_EYE), 0, FILTER_LINEAR);
		GetUtils().DrawFullScreenTri(width, height);
		NvAPI_Stereo_SetActiveEye(m_nvStereoHandle, NVAPI_STEREO_EYE_LEFT);
	}
#endif

	pSH->FXEndPass();
	pSH->FXEnd();

	m_renderer.RT_SetViewport(0, 0, m_renderer.m_backbufferWidth, m_renderer.m_backbufferHeight);
}

void CD3DStereoRenderer::BeginRenderingMRT(bool disableClear)
{
	if (!IsStereoEnabled())
		return;

	if (disableClear)
	{
		m_needClearLeft = false;
		m_needClearRight = false;
	}

	PushRenderTargets();
}

void CD3DStereoRenderer::EndRenderingMRT(bool bResolve)
{
	if (!IsStereoEnabled())
		return;

	PopRenderTargets(bResolve);
}

void CD3DStereoRenderer::TakeScreenshot(const char path[])
{
	stack_string pathL(path);
	stack_string pathR(path);

	pathL.insert(pathL.find('.'), "_L");
	pathR.insert(pathR.find('.'), "_R");

	((CD3D9Renderer*) gRenDev)->CaptureFrameBufferToFile(pathL.c_str(), GetEyeTarget(LEFT_EYE));
	((CD3D9Renderer*) gRenDev)->CaptureFrameBufferToFile(pathR.c_str(), GetEyeTarget(RIGHT_EYE));
}

void CD3DStereoRenderer::NotifyFrameFinished()
{
	m_bDisplayStereoDone = false;
}

void CD3DStereoRenderer::BeginRenderingTo(StereoEye eye)
{
	m_curEye = eye;

	if (eye == LEFT_EYE)
	{
		gRenDev->SetProfileMarker("LEFT_EYE", CRenderer::ESPM_PUSH);
		if (m_needClearLeft)
		{
			m_needClearLeft = false;
			m_renderer.FX_ClearTarget(GetEyeTarget(LEFT_EYE), Clr_Transparent);
		}
		m_renderer.FX_PushRenderTarget(0, GetEyeTarget(LEFT_EYE), &m_renderer.m_DepthBufferOrig);
	}
	else
	{
		gRenDev->SetProfileMarker("RIGHT_EYE", CRenderer::ESPM_PUSH);
		if (m_needClearRight)
		{
			m_needClearRight = false;
			m_renderer.FX_ClearTarget(GetEyeTarget(RIGHT_EYE), Clr_Transparent);
		}
		m_renderer.FX_PushRenderTarget(0, GetEyeTarget(RIGHT_EYE), &m_renderer.m_DepthBufferOrig);
	}

	m_renderer.FX_SetActiveRenderTargets();

	if (eye == LEFT_EYE)
		GetEyeTarget(LEFT_EYE)->SetResolved(true);
	else
		GetEyeTarget(RIGHT_EYE)->SetResolved(true);
}

void CD3DStereoRenderer::EndRenderingTo(StereoEye eye)
{
	if (eye == LEFT_EYE)
	{
		gRenDev->SetProfileMarker("LEFT_EYE", CRenderer::ESPM_POP);
		GetEyeTarget(LEFT_EYE)->SetResolved(true);
		m_renderer.FX_PopRenderTarget(0);
	}
	else
	{
		gRenDev->SetProfileMarker("RIGHT_EYE", CRenderer::ESPM_POP);
		GetEyeTarget(RIGHT_EYE)->SetResolved(true);
		m_renderer.FX_PopRenderTarget(0);
	}
}

void CD3DStereoRenderer::BeginRenderingToVrQuadLayer(RenderLayer::EQuadLayers id)
{
	CTexture* pTexture = GetVrQuadLayerTex(id);
	if (pTexture && m_pHmdRenderer)
	{
		if (RenderLayer::CProperties* pLayer = m_pHmdRenderer->GetQuadLayerProperties(id))
		{
			pLayer->SetActive(true);
		}

		gRenDev->SetProfileMarker("STEREO_QUAD", CRenderer::ESPM_PUSH);
		if (m_needClearVrQuadLayer)
		{
			m_renderer.FX_ClearTarget(pTexture, Clr_Transparent);
			m_needClearVrQuadLayer = false;
		}
		m_renderer.FX_PushRenderTarget(0, pTexture, &m_renderer.m_DepthBufferOrig);

		m_renderer.FX_SetActiveRenderTargets();

		pTexture->SetResolved(true);
	}

}

void CD3DStereoRenderer::EndRenderingToVrQuadLayer(RenderLayer::EQuadLayers id)
{
	CTexture* pTexture = GetVrQuadLayerTex(id);
	if (pTexture)
	{
		gRenDev->SetProfileMarker("STEREO_QUAD", CRenderer::ESPM_POP);
		pTexture->SetResolved(true);
		m_renderer.FX_PopRenderTarget(0);
	}
}

void CD3DStereoRenderer::SelectShaderTechnique()
{
	gRenDev->m_cEF.mfRefreshSystemShader("Stereo", CShaderMan::s_ShaderStereo);

	CShader* pSH = m_renderer.m_cEF.s_ShaderStereo;

	if (m_device == STEREO_DEVICE_FRAMECOMP)
	{
		switch (GetStereoOutput())
		{
		case STEREO_OUTPUT_SIDE_BY_SIDE_SQUEEZED:
			pSH->FXSetTechnique("SideBySide");
			break;
		case STEREO_OUTPUT_CHECKERBOARD:
			pSH->FXSetTechnique("Checkerboard");
			break;
		case STEREO_OUTPUT_SIDE_BY_SIDE:
			pSH->FXSetTechnique("SideBySide");
			break;
		case STEREO_OUTPUT_LINE_BY_LINE:
			pSH->FXSetTechnique("LineByLine");
			break;
#ifndef _RELEASE
		case STEREO_OUTPUT_ANAGLYPH:
			pSH->FXSetTechnique("Anaglyph");
			break;
#endif
		default:
			pSH->FXSetTechnique("Emulation");
		}
	}
	else if (IsDriver(DRIVER_NV))
	{
		pSH->FXSetTechnique("NV3DVision");
	}
	else
	{
		pSH->FXSetTechnique("Emulation");
	}
}

void CD3DStereoRenderer::RenderScene(int sceneFlags, const SRenderingPassInfo& passInfo)
{
	m_renderer.EF_Scene3D(m_renderer.m_MainRTViewport, sceneFlags, passInfo);
}

bool CD3DStereoRenderer::IsRenderThread() const
{
	return m_renderer.m_pRT->IsRenderThread();
}

void CD3DStereoRenderer::CopyToStereoFromMainThread(int channel)
{
	m_renderer.m_pRT->RC_CopyToStereoTex(channel);
}

void CD3DStereoRenderer::PushRenderTargets()
{
	if (m_needClearLeft)
	{
		m_needClearLeft = false;
		m_renderer.FX_ClearTarget(GetEyeTarget(LEFT_EYE), Clr_Transparent);
	}

	if (m_needClearRight)
	{
		m_needClearRight = false;
		m_renderer.FX_ClearTarget(GetEyeTarget(RIGHT_EYE), Clr_Transparent);
	}

	m_renderer.FX_PushRenderTarget(0, GetEyeTarget(LEFT_EYE), &m_renderer.m_DepthBufferOrig);
	m_renderer.FX_PushRenderTarget(1, GetEyeTarget(RIGHT_EYE), NULL);

	m_renderer.RT_SetViewport(0, 0, GetEyeTarget(LEFT_EYE)->GetWidth(), GetEyeTarget(LEFT_EYE)->GetHeight());

	m_renderer.FX_SetActiveRenderTargets();
}

void CD3DStereoRenderer::PopRenderTargets(bool bResolve)
{
	m_renderer.FX_PopRenderTarget(1);
	m_renderer.FX_PopRenderTarget(0);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
// IStereoRenderer Interface
////////////////////////////////////////////////////////////////////////////////////////////////////

EStereoDevice CD3DStereoRenderer::GetDevice()
{
	return m_device;
}

EStereoDeviceState CD3DStereoRenderer::GetDeviceState()
{
	return m_deviceState;
}

void CD3DStereoRenderer::GetInfo(EStereoDevice* device, EStereoMode* mode, EStereoOutput* output, EStereoDeviceState* state)
{
	if (device) *device = m_device;
	if (mode) *mode = m_mode;
	if (output) *output = m_output;
	if (state) *state = m_deviceState;
}

bool CD3DStereoRenderer::GetStereoEnabled()
{
	return IsStereoEnabled();
}

float CD3DStereoRenderer::GetStereoStrength()
{
	return m_stereoStrength;
}

float CD3DStereoRenderer::GetMaxSeparationScene(bool half)
{
	return m_maxSeparationScene * (half ? 0.5f : 1.0f);
}

float CD3DStereoRenderer::GetZeroParallaxPlaneDist()
{
	return m_zeroParallaxPlaneDist;
}

void CD3DStereoRenderer::GetNVControlValues(bool& stereoActivated, float& stereoStrength)
{
	stereoActivated = m_nvStereoActivated != 0;
	stereoStrength = m_nvStereoStrength;
}

void CD3DStereoRenderer::ReleaseBuffers()
{
	if (m_pHmdRenderer != nullptr)
		m_pHmdRenderer->ReleaseBuffers();
}

void CD3DStereoRenderer::OnResolutionChanged()
{
	// StereoL and StereoR buffers are used as temporary buffers in other passes and always required
	CreateIntermediateBuffers();

	if (m_device == STEREO_DEVICE_NONE)
		return;

	if (m_pHmdRenderer != nullptr)
	{
		m_pHmdRenderer->OnResolutionChanged();
	}
}

void CD3DStereoRenderer::CalculateBackbufferResolution(int eyeWidth, int eyeHeight, int* pBackbufferWidth, int* pBackbufferHeight)
{
	if (m_pHmdRenderer != nullptr)
		m_pHmdRenderer->CalculateBackbufferResolution(eyeWidth, eyeHeight, pBackbufferWidth, pBackbufferHeight);
	else
	{
		switch (m_output)
		{
		case STEREO_OUTPUT_SIDE_BY_SIDE:
			*pBackbufferWidth = eyeWidth * 2;
			*pBackbufferHeight = eyeHeight;
			break;
		case STEREO_OUTPUT_ABOVE_AND_BELOW:
			*pBackbufferWidth = eyeWidth;
			*pBackbufferHeight = eyeHeight * 2;
			break;
		default:
			*pBackbufferWidth = eyeWidth;
			*pBackbufferHeight = eyeHeight;
			break;
		}
	}
}

void CD3DStereoRenderer::OnHmdDeviceChanged(IHmdDevice* pHmdDevice)
{
	static bool firstInitialization = true;

	ShutdownHmdRenderer();

	m_pHmdDevice = pHmdDevice;

	InitializeHmdRenderer();

	//Do an initial initialization for the hmd renderer when not activated as output so it can setup it's state if needed (at least osvr needs to initialize the renderer before it can be queried for the preferred resolution)
	if (m_pHmdDevice && !m_pHmdRenderer && m_output != STEREO_OUTPUT_HMD && firstInitialization)
	{
		m_pHmdRenderer = CreateHmdRenderer(STEREO_OUTPUT_HMD, m_pHmdDevice, &m_renderer, this);

		if (m_pHmdRenderer != nullptr)
		{
			m_pHmdRenderer->Initialize();
		}
		ShutdownHmdRenderer();
	}

	firstInitialization = false;

}

IHmdRenderer* CD3DStereoRenderer::CreateHmdRenderer(EStereoOutput stereoOutput, IHmdDevice* pDevice, CD3D9Renderer* pRenderer, CD3DStereoRenderer* pStereoRenderer)
{
	if (pDevice)
	{
		const EHmdClass deviceClass = pDevice->GetClass();
		switch (deviceClass)
		{
#if defined(INCLUDE_OCULUS_SDK)
		case eHmdClass_Oculus:
			{
				if (stereoOutput == STEREO_OUTPUT_HMD && pDevice != nullptr && pDevice->GetClass() == eHmdClass_Oculus)
					return new CD3DOculusRenderer(static_cast<CryVR::Oculus::IOculusDevice*>(pDevice), pRenderer, pStereoRenderer);
			}
			break;
#endif
#if defined(INCLUDE_OPENVR_SDK)
		case eHmdClass_OpenVR:
			if (stereoOutput == STEREO_OUTPUT_HMD && pDevice != NULL && pDevice->GetClass() == eHmdClass_OpenVR)
				return new CD3DOpenVRRenderer(static_cast<CryVR::OpenVR::IOpenVRDevice*>(pDevice), pRenderer, pStereoRenderer);
			break;
#endif
#if defined(INCLUDE_OSVR_SDK)
		case eHmdClass_Osvr:
			if (stereoOutput == STEREO_OUTPUT_HMD && pDevice != NULL && pDevice->GetClass() == eHmdClass_Osvr)
				return new CryVR::Osvr::CD3DOsvrRenderer(static_cast<CryVR::Osvr::IOsvrDevice*>(pDevice), pRenderer, pStereoRenderer);
#endif
		case eHmdClass_Null:
		default:
			break;
		}
	}
	return nullptr;
}

//////////////////////////////////////////////////////////////////////////
void CD3DStereoRenderer::TryInjectHmdCameraAsync(CRenderView* pRenderView)
{
	IHmdManager* pHmdManager = gEnv->pSystem->GetHmdManager();
	if (!pHmdManager)
		return;

	IHmdDevice* pHmdDevice = pHmdManager->GetHmdDevice();
	if (!pHmdDevice)
		return;

	const HmdTrackingState& trackingState = pHmdDevice->GetLocalTrackingState();
	if (!trackingState.CheckStatusFlags(eHmdStatus_IsUsable))
		return;

	if (m_renderer.m_CurRenderEye == LEFT_EYE)
	{
		m_bAsyncCameraMatrixValid = false;
		// Only query async tracking state for the first left eye.
		IHmdDevice::AsyncCameraContext context;
		context.frameId = gRenDev->GetFrameID(false);
		if (!pHmdDevice->RequestAsyncCameraUpdate(context))
			return;
		m_bAsyncCameraMatrixValid = true;
		m_asyncCameraMatrix = context.outputCameraMatrix;
	}

	if (!m_bAsyncCameraMatrixValid)
		return;

	// new up to date matrix is received, use it for current frame
	CCamera currentCamera = m_renderer.GetCamera();
	currentCamera.SetMatrix(m_asyncCameraMatrix);
	CCamera newCamera = PrepareCamera(m_renderer.m_CurRenderEye, currentCamera);
	m_renderer.SetCamera(newCamera);
}
