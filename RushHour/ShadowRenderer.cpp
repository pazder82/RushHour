#include "stdafx.h"
#include <d3dcompiler.h>
#include "ShadowRenderer.h"
#include "CommonException.h"

void ShadowRenderer::LoadRenderTextureShaders() {
	// load and compile vertex and pixel shader
	ID3D10Blob *VS, *PS;
	D3DCompileFromFile(L"shadow.shader", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VShader", "vs_4_0", 0, 0, &VS, NULL);
	D3DCompileFromFile(L"shadow.shader", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PShader", "ps_4_0", 0, 0, &PS, NULL);
	if (FAILED(_d3d->_dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &_rtvs))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D vertex shader!");
	}
	if (FAILED(_d3d->_dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &_rtps))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D pixel shader!");
	}

	// create the input layout object
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	if (FAILED(_d3d->_dev->CreateInputLayout(ied, 3, VS->GetBufferPointer(), VS->GetBufferSize(), &_rtlayout))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D input layout!");
	}
}
