// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

#include "StdAfx.h"
#include "CryDX12.hpp"

#include "GI/CCryDX12GIFactory.hpp"
#include "Device/CCryDX12Device.hpp"
#include "Device/CCryDX12DeviceContext.hpp"

HRESULT WINAPI DX12CreateDXGIFactory1(REFIID riid, void** ppFactory)
{
	*ppFactory = CCryDX12GIFactory::Create();
	return *ppFactory ? 0 : -1;
}

HRESULT WINAPI DX12CreateDevice(
  IDXGIAdapter* pAdapter,
  D3D_DRIVER_TYPE DriverType,
  HMODULE Software,
  UINT Flags,
  CONST D3D_FEATURE_LEVEL* pFeatureLevels,
  UINT FeatureLevels,
  UINT SDKVersion,
  ID3D11Device** ppDevice,
  D3D_FEATURE_LEVEL* pFeatureLevel,
  ID3D11DeviceContext** ppImmediateContext)
{
	*ppDevice = CCryDX12Device::Create(pAdapter, pFeatureLevel);

	if (!*ppDevice)
	{
		return -1;
	}

	(*ppDevice)->GetImmediateContext(ppImmediateContext);

	if (!*ppImmediateContext)
	{
		return -1;
	}

	return 0;
}
