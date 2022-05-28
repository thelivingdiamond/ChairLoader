// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:
//  Version:     v1.00
//  Created:     08/05/2015 by Jan Pinter
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef __DX12SAMPLERSTATE__
	#define __DX12SAMPLERSTATE__

	#include "DX12Base.hpp"

namespace NCryDX12
{

class CSamplerState : public CRefCounted
{
public:
	CSamplerState();
	virtual ~CSamplerState();

	D3D12_SAMPLER_DESC& GetSamplerDesc()
	{
		//		DX12_ASSERT(m_pResource && (m_pResource->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL));
		return m_unSamplerDesc;
	}
	const D3D12_SAMPLER_DESC& GetSamplerDesc() const
	{
		//		DX12_ASSERT(m_pResource && (m_pResource->GetDesc().Flags & D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL));
		return m_unSamplerDesc;
	}

	void                        SetDescriptorHandle(const D3D12_CPU_DESCRIPTOR_HANDLE& descriptorHandle) { m_DescriptorHandle = descriptorHandle; }
	D3D12_CPU_DESCRIPTOR_HANDLE GetDescriptorHandle() const                                              { return m_DescriptorHandle; }

private:
	D3D12_CPU_DESCRIPTOR_HANDLE m_DescriptorHandle;

	union
	{
		D3D12_SAMPLER_DESC m_unSamplerDesc;
	};
};

}

#endif // __DX12SAMPLERSTATE__
