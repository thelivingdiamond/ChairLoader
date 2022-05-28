// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"

#if defined(INCLUDE_OPENVR_SDK)

	#include "D3DOpenVR.h"
	#include "DriverD3D.h"
	#include "D3DPostProcess.h"
	#include "DeviceInfo.h"

	#include <CrySystem/VR/IHMDManager.h>
	#include <CrySystem/VR/IHMDDevice.h>
	#ifdef ENABLE_BENCHMARK_SENSOR
		#include <IBenchmarkFramework.h>
		#include <IBenchmarkRendererSensorManager.h>
	#endif

CD3DOpenVRRenderer::CD3DOpenVRRenderer(CryVR::OpenVR::IOpenVRDevice* openVRDevice, CD3D9Renderer* renderer, CD3DStereoRenderer* stereoRenderer)
	: m_numFrames(0)
	, m_currentFrame(0)
	, m_mirrorTexture(nullptr)
	, m_uEyeWidth(~0L)
	, m_uEyeHeight(~0L)
	, m_openVRDevice(openVRDevice)
	, m_renderer(renderer)
	, m_stereoRenderer(stereoRenderer)
	, m_backbufferTexture(nullptr)
	, m_backbuffer(nullptr)
{
	ZeroArray(m_eyes);
	ZeroArray(m_quads);

	for (uint32 i = RenderLayer::eQuadLayers_0; i < RenderLayer::eQuadLayers_Total; ++i)
	{
		m_quadLayerProperties[i].SetType(RenderLayer::eLayer_Quad);
		m_quadLayerProperties[i].SetPose(QuatTS(Quat(IDENTITY), Vec3(0.f, 0.f, -0.6f), 1.f));
		m_quadLayerProperties[i].SetId(i);
	}
}

CD3DOpenVRRenderer::~CD3DOpenVRRenderer()
{
}

bool CD3DOpenVRRenderer::Initialize()
{
	D3DDevice* d3d11Device = m_renderer->GetDevice_Unsynchronized().GetRealDevice();

	//int w = 0, h = 0;
	//m_openVRDevice->GetRenderTargetSize(&w, &h);
	//m_uEyeWidth = w;
	//m_uEyeHeight = h;
	// we want more control over the render resolution, therefore we use the following - at-least for now -
	m_uEyeWidth = m_renderer->GetWidth();
	m_uEyeHeight = m_renderer->GetHeight();

	m_Param0Name = CCryNameR("texToTexParams0");
	m_Param1Name = CCryNameR("texToTexParams1");
	m_textureToTexture = CCryNameTSCRC("TextureToTexture");

	TextureDesc eyeTextureDesc;
	eyeTextureDesc.width = m_uEyeWidth;
	eyeTextureDesc.height = m_uEyeHeight;
	eyeTextureDesc.format = (uint32)DXGI_FORMAT_R8G8B8A8_UNORM;

	TextureDesc mirrorTextureDesc;
	mirrorTextureDesc.width = m_uEyeWidth * 2;
	mirrorTextureDesc.height = m_uEyeHeight;
	mirrorTextureDesc.format = (uint32)DXGI_FORMAT_R8G8B8A8_UNORM;

	TextureDesc quadTextureDesc;
	quadTextureDesc.width = m_uEyeWidth;
	quadTextureDesc.height = m_uEyeHeight;
	quadTextureDesc.format = (uint32)DXGI_FORMAT_R8G8B8A8_UNORM;

	if (!InitializeEyeTarget(d3d11Device, EEyeType::eEyeType_LeftEye, eyeTextureDesc, "$LeftEye") ||
	    !InitializeEyeTarget(d3d11Device, EEyeType::eEyeType_RightEye, eyeTextureDesc, "$RightEye") ||
	    !InitializeMirrorTexture(d3d11Device, mirrorTextureDesc, "$StereoMirror"))
	{
		CryLogAlways("[HMD][OpenVR] Texture creation failed");
		Shutdown();
		return false;
	}
	for (int i = 0; i < RenderLayer::eQuadLayers_Total; i++)
	{
		if (!InitializeQuadLayer(d3d11Device, i, quadTextureDesc, "$Quad_%d"))
		{
			CryLogAlways("[HMD][OpenVR] Texture creation failed");
			Shutdown();
			return false;
		}
	}

	m_openVRDevice->OnSetupEyeTargets(
	  CryVR::OpenVR::ERenderAPI::eRenderAPI_DirectX,
	  CryVR::OpenVR::ERenderColorSpace::eRenderColorSpace_Auto,
	  m_eyes[EEyeType::eEyeType_LeftEye].texture->GetDevTexture()->Get2DTexture(),
	  m_eyes[EEyeType::eEyeType_RightEye].texture->GetDevTexture()->Get2DTexture()
	  );

	for (int i = 0; i < RenderLayer::eQuadLayers_Total; i++)
		m_openVRDevice->OnSetupOverlay(i, CryVR::OpenVR::ERenderAPI::eRenderAPI_DirectX, CryVR::OpenVR::ERenderColorSpace::eRenderColorSpace_Auto, m_quads[i].texture->GetDevTexture()->Get2DTexture());

	UpdateTargetBuffer();

	return true;
}

bool CD3DOpenVRRenderer::InitializeEyeTarget(D3DDevice* d3d11Device, EEyeType eye, TextureDesc desc, const char* name)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = desc.width;
	textureDesc.Height = desc.height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = (DXGI_FORMAT)desc.format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;
	ID3D11Texture2D* texture;
	d3d11Device->CreateTexture2D(&textureDesc, nullptr, &texture);

	ETEX_Format format = CTexture::TexFormatFromDeviceFormat((DXGI_FORMAT)desc.format);
	m_eyes[eye].texture = WrapD3DRenderTarget(static_cast<D3DTexture*>(texture), desc.width, desc.height, format, name, true);

	return true;
}

bool CD3DOpenVRRenderer::InitializeQuadLayer(D3DDevice* d3d11Device, int quadLayer, TextureDesc desc, const char* name)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = desc.width;
	textureDesc.Height = desc.height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = (DXGI_FORMAT)desc.format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_SHARED;
	ID3D11Texture2D* texture;
	d3d11Device->CreateTexture2D(&textureDesc, nullptr, &texture);

	char textureName[16];
	cry_sprintf(textureName, name, quadLayer);

	ETEX_Format format = CTexture::TexFormatFromDeviceFormat((DXGI_FORMAT)desc.format);
	m_quads[quadLayer].texture = WrapD3DRenderTarget(static_cast<D3DTexture*>(texture), desc.width, desc.height, format, textureName, true);

	return true;
}

bool CD3DOpenVRRenderer::InitializeMirrorTexture(D3DDevice* d3d11Device, TextureDesc desc, const char* name)
{
	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Width = desc.width;
	textureDesc.Height = desc.height;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = (DXGI_FORMAT)desc.format;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	ID3D11Texture2D* texture;
	d3d11Device->CreateTexture2D(&textureDesc, nullptr, &texture);

	ETEX_Format format = CTexture::TexFormatFromDeviceFormat((DXGI_FORMAT)desc.format);
	m_mirrorTexture = WrapD3DRenderTarget(static_cast<D3DTexture*>(texture), desc.width, desc.height, format, name, true);

	if (m_mirrorTexture == nullptr)
		return false;

	return true;
}

void CD3DOpenVRRenderer::Shutdown()
{
	m_stereoRenderer->SetEyeTextures(nullptr, nullptr);

	for (uint32 eye = 0; eye < 2; ++eye)
	{
		m_eyes[eye].texture->ReleaseForce();
		m_eyes[eye].texture = nullptr;
	}

	for (int i = 0; i < RenderLayer::eQuadLayers_Total; i++)
	{
		m_openVRDevice->OnDeleteOverlay(i);
		m_quads[i].texture->ReleaseForce();
		m_quads[i].texture = nullptr;
	}

	if (m_mirrorTexture != nullptr)
		m_mirrorTexture->ReleaseForce();

	ReleaseBuffers();
}

void CD3DOpenVRRenderer::CalculateBackbufferResolution(int eyeWidth, int eyeHeight, int* pBackbufferWidth, int* pBackbufferHeight)
{
	*pBackbufferWidth = 2 * eyeWidth;
	*pBackbufferHeight = eyeHeight;
}

void CD3DOpenVRRenderer::OnResolutionChanged()
{
	if (m_uEyeWidth != m_renderer->GetWidth() ||
	    m_uEyeHeight != m_renderer->GetHeight())
	{
		Shutdown();
		Initialize();
	}
	else
	{
		UpdateTargetBuffer();
	}
}

void CD3DOpenVRRenderer::ReleaseBuffers()
{
	SAFE_RELEASE(m_backbufferTexture);
	SAFE_RELEASE(m_backbuffer);
}

void CD3DOpenVRRenderer::PrepareFrame()
{
	m_stereoRenderer->SetEyeTextures(m_eyes[0].texture, m_eyes[1].texture);
	for (int i = 0; i < RenderLayer::eQuadLayers_Total; i++)
		m_stereoRenderer->SetVrQuadLayerTexture(static_cast<RenderLayer::EQuadLayers>(i), m_quads[i].texture);
}

void CD3DOpenVRRenderer::SubmitFrame()
{
	#ifdef ENABLE_BENCHMARK_SENSOR
	gcpRendD3D->m_benchmarkRendererSensor->PreStereoFrameSubmit(m_eyes[0].texture, m_eyes[1].texture);
	#endif
	for (int id = 0; id < RenderLayer::eQuadLayers_Total; id++)
		if (GetQuadLayerProperties(static_cast<RenderLayer::EQuadLayers>(id))->IsActive())
			m_openVRDevice->SubmitOverlay(id);

	m_openVRDevice->SubmitFrame();
	#ifdef ENABLE_BENCHMARK_SENSOR
	gcpRendD3D->m_benchmarkRendererSensor->AfterStereoFrameSubmit();
	#endif

	GetQuadLayerProperties(RenderLayer::eQuadLayers_0)->SetActive(false);
	GetQuadLayerProperties(RenderLayer::eQuadLayers_1)->SetActive(false);
}

void CD3DOpenVRRenderer::RenderSocialScreen()
{
	if (const IHmdManager* pHmdManager = gEnv->pSystem->GetHmdManager())
	{
		if (const IHmdDevice* pDev = pHmdManager->GetHmdDevice())
		{
			const EHmdSocialScreen socialScreen = pDev->GetSocialScreenType();
			switch (socialScreen)
			{
			case EHmdSocialScreen::eHmdSocialScreen_Off:
				{
					GetUtils().ClearScreen(0.1f, 0.1f, 0.1f, 1.0f); // we don't want true black, to distinguish between rendering error and no-social-screen. NOTE: THE CONSOLE WILL NOT BE DISPLAYED!!!
				}
				break;
			// intentional fall through
			case EHmdSocialScreen::eHmdSocialScreen_UndistortedLeftEye:
			case EHmdSocialScreen::eHmdSocialScreen_UndistortedRightEye:
				{
					CTexture* pTex = socialScreen == EHmdSocialScreen::eHmdSocialScreen_UndistortedLeftEye ? m_stereoRenderer->GetEyeTarget(LEFT_EYE) : m_stereoRenderer->GetEyeTarget(RIGHT_EYE);
					if (CShaderMan::s_shPostEffects)
						GetUtils().StretchRect(pTex, m_backbufferTexture);
				}
				break;

			case EHmdSocialScreen::eHmdSocialScreen_UndistortedDualImage: // intentional fall through - default to undistorted dual image
			case EHmdSocialScreen::eHmdSocialScreen_DistortedDualImage:   // intentional fall through - OpenVR does not return distorted eye targets, therefore the only display the undistorted eye targets
			default:
				if (CShaderMan::s_shPostEffects)
				{
					// Get eye textures
					CTexture* pTexLeft = m_stereoRenderer->GetEyeTarget(LEFT_EYE);
					CTexture* pTexRight = m_stereoRenderer->GetEyeTarget(RIGHT_EYE);

					// Store current viewport
					int iTempX, iTempY, iWidth, iHeight;
					gRenDev->GetViewport(&iTempX, &iTempY, &iWidth, &iHeight);

					// Set backbuffer texture as renter target
					gcpRendD3D->FX_PushRenderTarget(0, m_backbufferTexture, nullptr);
					gcpRendD3D->FX_SetActiveRenderTargets();

					// Set rendering and shader flags
					gcpRendD3D->FX_SetState(GS_NODEPTHTEST);

					uint64 shaderFlags = gRenDev->m_RP.m_FlagsShader_RT;

					gRenDev->m_RP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_SAMPLE0];
					gRenDev->m_RP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_SAMPLE2];
					gRenDev->m_RP.m_FlagsShader_RT &= ~g_HWSR_MaskBit[HWSR_SAMPLE5];

					// Left-Eye Pass
					gcpRendD3D->RT_SetViewport(0, 0, m_backbufferTexture->GetWidth() >> 1, m_backbufferTexture->GetHeight()); // Set viewport (left half of backbuffer)

					uint nPasses;
					CShaderMan::s_shPostEffects->FXSetTechnique(m_textureToTexture); // TextureToTexture technique
					CShaderMan::s_shPostEffects->FXBegin(&nPasses, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
					CShaderMan::s_shPostEffects->FXBeginPass(0);
					pTexLeft->Apply(0, CTexture::GetTexState(STexState(FILTER_LINEAR, true)), EFTT_UNKNOWN, -1, SResourceView::DefaultView); // Bind left-eye texture for rendering
					GetUtils().DrawFullScreenTri(pTexLeft->GetWidth(), pTexLeft->GetHeight(), 0);                                            // render
					CShaderMan::s_shPostEffects->FXEndPass();
					CShaderMan::s_shPostEffects->FXEnd();

					// Right-Eye Pass
					gcpRendD3D->RT_SetViewport(m_backbufferTexture->GetWidth() >> 1, 0, m_backbufferTexture->GetWidth() >> 1, m_backbufferTexture->GetHeight()); // set viewport (right half of backbuffer)

					CShaderMan::s_shPostEffects->FXSetTechnique(m_textureToTexture); // TextureToTexture technique
					CShaderMan::s_shPostEffects->FXBegin(&nPasses, FEF_DONTSETTEXTURES | FEF_DONTSETSTATES);
					CShaderMan::s_shPostEffects->FXBeginPass(0);
					pTexRight->Apply(0, CTexture::GetTexState(STexState(FILTER_LINEAR, true)), EFTT_UNKNOWN, -1, SResourceView::DefaultView); // Bind right-eye texture for rendering
					GetUtils().DrawFullScreenTri(pTexRight->GetWidth(), pTexRight->GetHeight(), 1);                                           // render
					CShaderMan::s_shPostEffects->FXEndPass();
					CShaderMan::s_shPostEffects->FXEnd();

					// Undo renderer changes (flags & render target)
					gRenDev->m_RP.m_FlagsShader_RT = shaderFlags;

					gcpRendD3D->FX_PopRenderTarget(0);
					gcpRendD3D->SetViewport(iTempX, iTempY, iWidth, iHeight);
				}
				break;
			}
		}
	}
}

CTexture* CD3DOpenVRRenderer::WrapD3DRenderTarget(D3DTexture* d3dTexture, uint32 width, uint32 height, ETEX_Format format, const char* name, bool shaderResourceView)
{
	CTexture* texture = CTexture::CreateTextureObject(name, width, height, 1, eTT_2D, FT_DONT_STREAM | FT_USAGE_RENDERTARGET, format);
	if (texture == nullptr)
	{
		gEnv->pLog->Log("[HMD][OpenVR] Unable to create texture object!");
		return nullptr;
	}

	// CTexture::CreateTextureObject does not set width and height if the texture already existed
	assert(texture->GetWidth() == width);
	assert(texture->GetHeight() == height);
	assert(texture->GetDepth() == 1);

	d3dTexture->AddRef();
	CDeviceTexture* deviceTexture = new CDeviceTexture(d3dTexture);
	deviceTexture->SetNoDelete(true);

	texture->SetDevTexture(deviceTexture);
	texture->ClosestFormatSupported(format);

	if (shaderResourceView)
	{
		void* default_srv = texture->GetResourceView(SResourceView::ShaderResourceView(format, 0, -1, 0, 1, false, false)).m_pDeviceResourceView;
		if (default_srv == nullptr)
		{
			gEnv->pLog->Log("[HMD][OpenVR] Unable to create default shader resource view!");
			texture->Release();
			return nullptr;
		}
		texture->SetShaderResourceView((D3DShaderResource*)default_srv, false);
	}

	return texture;
}

void CD3DOpenVRRenderer::UpdateTargetBuffer()
{
	SAFE_RELEASE(m_backbuffer);
	SAFE_RELEASE(m_backbufferTexture);
	DeviceInfo& devinfo = m_renderer->DevInfo();
	devinfo.BackbufferRTV()->GetResource(&m_backbuffer);
	if (m_backbuffer == nullptr)
	{
		gEnv->pLog->Log("[HMD][OpenVR] Could not retrieve the backbuffer!");
		return;
	}

	const DXGI_MODE_DESC& backbufferDesc(devinfo.SwapChainDesc().BufferDesc);
	uint32 backbufferWidth = backbufferDesc.Width;
	uint32 backbufferHeight = backbufferDesc.Height;
	ETEX_Format backbufferFormat = CTexture::TexFormatFromDeviceFormat(backbufferDesc.Format);

	m_backbufferTexture = WrapD3DRenderTarget((ID3D11Texture2D*)m_backbuffer, backbufferWidth, backbufferHeight, backbufferFormat, "$StereoMirrorBackbuffer", false);
}

RenderLayer::CProperties* CD3DOpenVRRenderer::GetQuadLayerProperties(RenderLayer::EQuadLayers id)
{
	if (id < RenderLayer::eQuadLayers_Total)
		return &(m_quadLayerProperties[id]);

	return nullptr;
}

#endif //defined(INCLUDE_OPENVR_SDK)
