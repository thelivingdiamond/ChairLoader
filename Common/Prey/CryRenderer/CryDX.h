#pragma once
#include <d3d11.h>

#define Direct3D IDXGIAdapter
#define DXGIFactory      IDXGIFactory1
#define DXGIDevice       IDXGIDevice1
#define DXGIAdapter      IDXGIAdapter1
#define DXGIOutput       IDXGIOutput
#define DXGISwapChain    IDXGISwapChain
#define D3DDeviceContext ID3D11DeviceContext
#define D3DDevice        ID3D11Device
#define D3DVertexDeclaration ID3D11InputLayout
#define D3DVertexShader      ID3D11VertexShader
#define D3DPixelShader       ID3D11PixelShader
#define D3DResource          ID3D11Resource
#define D3DBaseTexture     ID3D11Resource
#define D3DLookupTexture     ID3D11Texture1D
#define D3DTexture           ID3D11Texture2D
#define D3DVolumeTexture     ID3D11Texture3D
#define D3DCubeTexture       ID3D11Texture2D
#define D3DVertexBuffer      ID3D11Buffer
#define D3DShaderResource    ID3D11ShaderResourceView
#define D3DUAV               ID3D11UnorderedAccessView
#define D3DIndexBuffer       ID3D11Buffer
#define D3DBuffer            ID3D11Buffer
#define D3DSurface           ID3D11RenderTargetView
#define D3DDepthSurface      ID3D11DepthStencilView
#define D3DBaseView          ID3D11View
#define D3DQuery             ID3D11Query
#define D3DViewPort          D3D11_VIEWPORT
#define D3DRectangle         D3D11_RECT
#define D3DFormat            DXGI_FORMAT
#define D3DPrimitiveType     D3D11_PRIMITIVE_TOPOLOGY
#define D3DBlob              ID3D10Blob
#define D3DSamplerState      ID3D11SamplerState
