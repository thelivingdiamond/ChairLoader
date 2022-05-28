// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:
//  Version:     v1.00
//  Created:     11/02/2015 by Jan Pinter
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef __CCRYDX12BUFFER__
	#define __CCRYDX12BUFFER__

	#include "DX12/Resource/CCryDX12Resource.hpp"
	#include "DX12/API/DX12View.hpp"

class CCryDX12Buffer : public CCryDX12Resource<ID3D11Buffer>
{
public:
	DX12_OBJECT(CCryDX12Buffer, CCryDX12Resource<ID3D11Buffer> );

	static CCryDX12Buffer* Create(CCryDX12Device* pDevice);
	static CCryDX12Buffer* Create(CCryDX12Device* pDevice, ID3D12Resource* pResource, D3D12_RESOURCE_STATES initialState);
	static CCryDX12Buffer* Create(CCryDX12Device* pDevice, CCryDX12SwapChain* pSwapChain, ID3D12Resource* pResource);
	static CCryDX12Buffer* Create(CCryDX12Device* pDevice, const D3D11_BUFFER_DESC* pDesc, const D3D11_SUBRESOURCE_DATA* pInitialData);

	virtual ~CCryDX12Buffer();

	ILINE UINT GetStructureByteStride() const
	{
		// NOTE: masking by (m_Desc11.MiscFlags & D3D11_RESOURCE_MISC_BUFFER_STRUCTURED) was done in the constructor, no need to check here
		return m_Desc11.StructureByteStride;
	}

	NCryDX12::CView& GetDX12View()
	{
		return m_DX12View;
	}

	const NCryDX12::CView& GetDX12View() const
	{
		return m_DX12View;
	}

	#pragma region /* ICryDX12Resource implementation */

	virtual EDX12ResourceType GetDX12ResourceType() const final
	{
		return eDX12RT_Buffer;
	}

	virtual void STDMETHODCALLTYPE GetType(_Out_ D3D11_RESOURCE_DIMENSION* pResourceDimension) final
	{
		if (pResourceDimension)
		{
			*pResourceDimension = D3D11_RESOURCE_DIMENSION_BUFFER;
		}
	}

	void MapDiscard();
	void CopyDiscard();

	#pragma endregion

	#pragma region /* ID3D11Buffer implementation */

	virtual void STDMETHODCALLTYPE GetDesc(
	  _Out_ D3D11_BUFFER_DESC* pDesc) final;

	#pragma endregion

protected:
	CCryDX12Buffer(CCryDX12Device* pDevice, const D3D11_BUFFER_DESC& desc11, ID3D12Resource* pResource, D3D12_RESOURCE_STATES eInitialState, const D3D12_RESOURCE_DESC& desc12, const D3D11_SUBRESOURCE_DATA* pInitialData = NULL, size_t numInitialData = 0);

private:
	D3D11_BUFFER_DESC m_Desc11;

	NCryDX12::CView   m_DX12View;
};

#endif // __CCRYDX12BUFFER__
