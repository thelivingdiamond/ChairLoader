// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#ifndef _OSVR_DEVICE_H_
#define _OSVR_DEVICE_H_
#ifdef INCLUDE_OSVR_SDK
	#pragma once

	#include "HMDCVars.h"
	#include <CrySystem/VR/IHMDDevice.h>
	#include <CrySystem/VR/IHmdOSVRDevice.h>

	#include <osvr/ClientKit/ContextC.h>
	#include <osvr/ClientKit/DisplayC.h>
	#include <osvr/ClientKit/InterfaceStateC.h>
	#include <osvr/RenderKit/RenderManagerC.h>
	#if CRY_PLATFORM_WINDOWS
		#include <osvr/RenderKit/RenderManagerD3D11C.h>
	#else
		#include <osvr/RenderKit/RenderManagerOpenGLC.h>
	#endif
	#include <CryRenderer/Tarray.h>
struct OSVR_PoseReport;
struct OSVR_ImagingReport;
struct OSVR_TimeValue;

namespace CryVR
{
namespace Osvr
{
class Device : public IOsvrDevice, public ISystemEventListener, public CryVR::IVRCmdListener
{
public:
	// IHMDDevice interface
	virtual void                    AddRef() override;
	virtual void                    Release() override;
	int                             GetRefCount() const       { return m_refCount; }

	virtual EHmdClass               GetClass() const override { return eHmdClass_Osvr; }
	virtual void                    GetDeviceInfo(HmdDeviceInfo& info) const override;

	virtual void                    GetCameraSetupInfo(float& fov, float& aspectRatioFactor) const override;
	virtual void                    GetAsymmetricCameraSetupInfo(int nEye, float& fov, float& aspectRatio, float& asymH, float& asymV, float& eyeDist) const override;

	virtual void                    DisableHMDTracking(bool disable) override;
	virtual void                    UpdateInternal(EInternalUpdate) override {};
	virtual void                    RecenterPose() override;
	virtual void                    UpdateTrackingState(EVRComponent) override;
	virtual const HmdTrackingState& GetNativeTrackingState() const override;
	virtual const HmdTrackingState& GetLocalTrackingState() const override;

	virtual const IHmdController*   GetController() const override;
	virtual const EHmdSocialScreen  GetSocialScreenType(bool* pKeepAspect = nullptr) const override;
	virtual void                    GetPreferredRenderResolution(unsigned int& width, unsigned int& height);
	virtual int                     GetControllerCount() const override { return 2; } // DARIO_SKYHARBOR: HACK. NOT RELEVANT FOR SKYHARBOR ANYWAY

	static Device*                  CreateAndInitializeDevice();

	//IOsvrDevice
	virtual bool InitializeRenderer(void* d3dDevice, void* d3dContext);
	virtual bool RegisterTextureSwapSet(TextureSwapSet* swapSet);
	virtual bool PresentTextureSet(int textureSetIndex);
	virtual void ShutdownRenderer();

	//ISystemEventListener
	virtual void OnSystemEvent(ESystemEvent event, UINT_PTR wparam, UINT_PTR lparam);

	// CryVR::IVRCmdListener
	virtual void OnCommand(CryVR::EVRCmd cmd, IConsoleCmdArgs* pArgs);
	// ~CryVR::IVRCmdListener

private:

	enum Eye
	{
		Left  = 0,
		Right = 1
	};

	struct EyeSetup
	{
		EyeSetup()
			: fov(1.853f),
			aspectRatio(2.f),
			asymV(0),
			asymH(0),
			eyeDist(0.064f)
		{}
		float fov;
		float aspectRatio;
		float asymV;
		float asymH;
		float eyeDist;
	};

	EyeSetup                               m_eyes[2];
	HmdTrackingState                       m_localTrackingState;
	HmdTrackingState                       m_nativeTrackingState;
	HmdTrackingState                       m_disabledTrackingState;
	HmdDeviceInfo                          m_deviceInfo;

	std::string                            m_model;
	std::string                            m_vendor;

	OSVR_ClientContext                     m_context;
	OSVR_ClientContext                     m_renderContext;
	OSVR_ClientInterface                   m_headInterface;
	OSVR_DisplayConfig                     m_displayConfig;
	OSVR_RenderManager                     m_renderManager;
	OSVR_RenderManagerD3D11                m_renderManagerD3D11;
	Quat                                   m_recenterQuat;
	OSVR_RenderParams                      m_renderParams;
	TArray<OSVR_RenderInfoD3D11>           m_latestRenderInfo;
	TArray<TArray<OSVR_RenderBufferD3D11>> m_renderBufferSets;
	float                                  m_maxFovY;
	float                                  m_maxAspect;
	volatile int                           m_refCount;
	int m_lastFrameTrackingStateUpdated;
	bool                                   m_disableTracking;
	bool                                   m_loadingScreenActive;

	Device();
	virtual ~Device();

	void Shutdown();

	bool InitializeDevice();
	void CalculateSymmetricalFovsFromDisplayConfig(float& fovH, float& fovV);
	bool UpdateEyeSetups();
	void UpdatePoseFromOsvrPose(const OSVR_PoseState& newPose);
	void UpdatePoseFromRenderInfo();
	void UpdatePose(const Quat& orientation, const Vec3& translation);
	void UpdateRenderInfo();

};
}

}
#endif

#endif
