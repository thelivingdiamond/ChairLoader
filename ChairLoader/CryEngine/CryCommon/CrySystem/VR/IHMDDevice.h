// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#pragma once

#include <CryInput/IInput.h>

enum EHmdClass
{
	eHmdClass_Null,
	eHmdClass_Oculus,
	eHmdClass_OpenVR,
	eHmdClass_Osvr
};

enum EHmdType
{
	eHmdType_Unknown,
	// OCULUS
	eHmdType_DK1,
	eHmdType_DKHD,
	eHmdType_DK2,
	eHmdType_CB,
	eHmdType_EVT,
	//Osvr
	eHmdType_HDK12,
	eHmdType_HDK13
};

enum EHmdStatus
{
	eHmdStatus_OrientationTracked = 0x0001,
	eHmdStatus_PositionTracked    = 0x0002,
	// Potentially deprecated since Oculus SDK 1.0 - Need to double check with HTC Vive and PS VR
	eHmdStatus_CameraPoseTracked  = 0x0004, // deprecated?
	eHmdStatus_PositionConnected  = 0x0008, // deprecated?
	eHmdStatus_HmdConnected       = 0x0010, // deprecated?
	//HS_ValidReferencePosition = 0x0020,

	eHmdStatus_IsUsable = /*eHmdStatus_HmdConnected | */ eHmdStatus_OrientationTracked
};

enum EEyeType
{
	eEyeType_LeftEye = 0,
	eEyeType_RightEye,
	eEyeType_NumEyes
};

struct HmdDeviceInfo
{
	HmdDeviceInfo()
		: type(eHmdType_Unknown)
		, productName(0)
		, manufacturer(0)
		, screenWidth(0)
		, screenHeight(0)
		, fovH(0)
		, fovV(0)
	{
	}

	EHmdType     type;

	const char*  productName;
	const char*  manufacturer;

	unsigned int screenWidth;
	unsigned int screenHeight;

	float        fovH;
	float        fovV;
};

enum class EHmdSocialScreen
{
	eHmdSocialScreen_Off                = -1,
	eHmdSocialScreen_DistortedDualImage = 0,
	eHmdSocialScreen_UndistortedDualImage,
	eHmdSocialScreen_UndistortedLeftEye,
	eHmdSocialScreen_UndistortedRightEye,

	// helpers
	eHmdSocialScreen_FirstInvalidIndex
};

struct HmdPoseState
{
	HmdPoseState()
		: orientation(Quat::CreateIdentity())
		, position(ZERO)
		, angularVelocity(ZERO)
		, linearVelocity(ZERO)
		, angularAcceleration(ZERO)
		, linearAcceleration(ZERO)
	{
	}

	Quat orientation;
	Vec3 position;
	Vec3 angularVelocity;
	Vec3 linearVelocity;
	Vec3 angularAcceleration;
	Vec3 linearAcceleration;
};

struct HmdTrackingState
{
	HmdTrackingState()
		: pose()
		, statusFlags(0)
	{
	}

	bool CheckStatusFlags(unsigned int checked, unsigned int wanted) const { return (statusFlags & checked) == wanted; }
	bool CheckStatusFlags(unsigned int checkedAndWanted) const             { return CheckStatusFlags(checkedAndWanted, checkedAndWanted); }

	HmdPoseState pose;

	unsigned int statusFlags;
};

enum EVRComponent
{
	eVRComponent_Hmd        = BIT(0),
	eVRComponent_Controller = BIT(1),
	eVRComponent_All        = eVRComponent_Hmd | eVRComponent_Controller
};

enum EHmdController
{
	// Oculus
	eHmdController_OculusLeftHand                 = 0,
	eHmdController_OculusRightHand                = 1,
	eHmdController_MaxNumOculusControllers        = 2,
	// OpenVR
	eHmdController_OpenVR_1                       = 0,
	eHmdController_OpenVR_2                       = 1,
	eHmdController_OpenVR_MaxNumOpenVRControllers = 2,
};

enum EHmdProjection
{
	eHmdProjection_Stereo = 0,
	eHmdProjection_Mono_Cinema,
	eHmdProjection_Mono_HeadLocked,
};

struct IHmdController
{
	typedef uint32 TLightColor;

	enum ECaps
	{
		eCaps_Buttons       = BIT(0),
		eCaps_Tracking      = BIT(1),
		eCaps_Sticks        = BIT(2),
		eCaps_Capacitors    = BIT(3),
		eCaps_Gestures      = BIT(4),
		eCaps_ForceFeedback = BIT(5),
		eCaps_Color         = BIT(6),
	};

	virtual bool IsConnected(EHmdController id) const = 0;

	// TODO: refactor FG node to keep track of the button state internally w/o using this functions.
	//! In general action listeners should be used instead of calling directly this functions.
	//! These functions are still used in FG nodes used for quick prototyping by design.
	//! \return State of the current buttons/triggers/gestures.
	virtual bool  IsButtonPressed(EHmdController id, EKeyId button)  const = 0;
	virtual bool  IsButtonTouched(EHmdController id, EKeyId button)  const = 0;
	virtual bool  IsGestureTriggered(EHmdController id, EKeyId gesture) const = 0;
	virtual float GetTriggerValue(EHmdController id, EKeyId trigger) const = 0;
	virtual Vec2  GetThumbStickValue(EHmdController id, EKeyId stick)   const = 0;

	//! \return Tracking state in the Hmds internal coordinates system.
	virtual const HmdTrackingState& GetNativeTrackingState(EHmdController controller)      const = 0;

	//! \return Tracking state in the Hmd's local tracking space using CRYENGINE's coordinate system.
	virtual const HmdTrackingState& GetLocalTrackingState(EHmdController controller) const = 0;

	//! Apply a signal to the force-feedback motors of the selected controller.
	virtual void ApplyForceFeedback(EHmdController id, float freq, float amplitude) = 0;

	//! Access to the light color of the some controllers.
	virtual void        SetLightColor(EHmdController id, TLightColor color) = 0;
	virtual TLightColor GetControllerColor(EHmdController id) const = 0;

	//! \return Capabilities of the selected controller.
	virtual uint32 GetCaps(EHmdController id) const = 0;

	//! \returns the hardware device handle(if applicable) of the selected controller
	virtual void* GetDeviceHandle(EHmdController id) const { return nullptr; };

protected:
	virtual ~IHmdController() {}
};

struct IHmdDevice
{
	enum EInternalUpdate
	{
		eInternalUpdate_DebugInfo = 0,
	};
	struct AsyncCameraContext
	{
		int      frameId;
		Matrix34 outputCameraMatrix;
	};
	struct IAsyncCameraCallback
	{
		// If return false, it is not possible to accurately retrieve new camera matrix.
		virtual bool OnAsyncCameraCallback(const HmdTrackingState& state, AsyncCameraContext& context) = 0;
	};

	virtual void      AddRef() = 0;
	virtual void      Release() = 0;

	virtual EHmdClass GetClass() const = 0;
	virtual void      GetDeviceInfo(HmdDeviceInfo& info) const = 0;

	virtual void      GetCameraSetupInfo(float& fov, float& aspectRatioFactor) const = 0;
	virtual void      GetAsymmetricCameraSetupInfo(int nEye, float& fov, float& aspectRatio, float& asymH, float& asymV, float& eyeDist) const = 0;

	virtual void      UpdateInternal(EInternalUpdate) = 0;
	virtual void RecenterPose() = 0;
	virtual void      UpdateTrackingState(EVRComponent) = 0;

	//! \return Tracking state in Hmd's internal coordinates system.
	virtual const HmdTrackingState& GetNativeTrackingState() const = 0;

	//! \return Tracking state in the Hmd's local tracking space using CRYENGINE's coordinate system.
	virtual const HmdTrackingState& GetLocalTrackingState() const = 0;

	virtual const IHmdController*   GetController() const = 0;
	virtual const EHmdSocialScreen  GetSocialScreenType(bool* pKeepAspect = nullptr) const = 0;

	virtual int                     GetControllerCount() const = 0;

	//useful for querying preferred rendering resolution for devices where the screen resolution is not the resolution the SDK wants for rendering (OSVR)
	virtual void GetPreferredRenderResolution(unsigned int& width, unsigned int& height) = 0;
	//Disables & Resets the tracking state to identity. Useful for benchmarking where we want the HMD to behave normally except that we want to force the direction of the camera.
	virtual void DisableHMDTracking(bool disable) = 0;

	// Assign a game side callback to be called asynchronously from any thread to update camera matrix
	virtual void SetAsynCameraCallback(IAsyncCameraCallback* pCallback) {};
	// Can be called from any thread to retrieve most up to date camera transformation
	virtual bool RequestAsyncCameraUpdate(AsyncCameraContext& context)  { return false; };
protected:
	virtual ~IHmdDevice() {}
};
