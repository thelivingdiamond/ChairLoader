// Copyright 2001-2016 Crytek GmbH / Crytek Group. All rights reserved.

// -------------------------------------------------------------------------
//  File name:
//  Version:     v1.00
//  Created:     15/02/2015 by Jan Pinter
//  Description:
// -------------------------------------------------------------------------
//  History:
//
////////////////////////////////////////////////////////////////////////////
#pragma once
#ifndef __CCRYDX12SHADER__
	#define __CCRYDX12SHADER__

	#include "DX12/Device/CCryDX12DeviceChild.hpp"
	#include "DX12/API/DX12Shader.hpp"

class CCryDX12Shader : public CCryDX12DeviceChild<ID3D11DeviceChild>
{
public:
	DX12_OBJECT(CCryDX12Shader, CCryDX12DeviceChild<ID3D11DeviceChild> );

	static CCryDX12Shader* Create(CCryDX12Device* pDevice, const void* pShaderBytecode, SIZE_T BytecodeLength, ID3D11ClassLinkage* pClassLinkage);

	virtual ~CCryDX12Shader();

	NCryDX12::CShader* GetDX12Shader() const
	{
		return m_pShader;
	}

	const D3D12_SHADER_BYTECODE& GetD3D12ShaderBytecode() const
	{
		return m_ShaderBytecode12;
	}

protected:
	CCryDX12Shader(const void* pShaderBytecode, SIZE_T BytecodeLength);

private:
	DX12_PTR(NCryDX12::CShader) m_pShader;

	uint8_t*              m_pShaderBytecodeData;

	D3D12_SHADER_BYTECODE m_ShaderBytecode12;
};

#endif // __CCRYDX12SHADER__
