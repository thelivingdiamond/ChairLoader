// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:
//  Version:     v1.00
//  Created:     03/02/2015 by Jan Pinter
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef __CCRYDX12DEPTHSTENCILSTATE__
	#define __CCRYDX12DEPTHSTENCILSTATE__

	#include "DX12/Device/CCryDX12DeviceChild.hpp"

class CCryDX12DepthStencilState : public CCryDX12DeviceChild<ID3D11DepthStencilState>
{
public:
	DX12_OBJECT(CCryDX12DepthStencilState, CCryDX12DeviceChild<ID3D11DepthStencilState> );

	static CCryDX12DepthStencilState* Create(const D3D11_DEPTH_STENCIL_DESC* pDepthStencilDesc);

	virtual ~CCryDX12DepthStencilState();

	const D3D12_DEPTH_STENCIL_DESC& GetD3D12DepthStencilDesc() const
	{
		return m_Desc12;
	}

	#pragma region /* ID3D11DepthStencilState implementation */

	virtual void STDMETHODCALLTYPE GetDesc(
	  _Out_ D3D11_DEPTH_STENCIL_DESC* pDesc);

	#pragma endregion

protected:
	CCryDX12DepthStencilState(const D3D11_DEPTH_STENCIL_DESC& desc11, const D3D12_DEPTH_STENCIL_DESC& desc12);

private:
	D3D11_DEPTH_STENCIL_DESC m_Desc11;
	D3D12_DEPTH_STENCIL_DESC m_Desc12;
};

#endif // __CCRYDX12DEPTHSTENCILSTATE__
