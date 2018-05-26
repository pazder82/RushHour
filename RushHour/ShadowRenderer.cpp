#include "stdafx.h"
#include <d3dcompiler.h>
#include "ShadowRenderer.h"
#include "CommonException.h"

void ShadowRenderer::LoadRenderTextureShaders() {
	// load and compile vertex and pixel shader
	ID3D10Blob *VS, *PS;
	D3DCompileFromFile(L"shadow.shader", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VShader", "vs_4_0", 0, 0, &VS, NULL);
	D3DCompileFromFile(L"shadow.shader", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PShader", "ps_4_0", 0, 0, &PS, NULL);
	if (FAILED(_d3d->GetDevice()->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &_rtvs))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D vertex shader!");
	}
	if (FAILED(_d3d->GetDevice()->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &_rtps))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D pixel shader!");
	}

	// create the input layout object
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	if (FAILED(_d3d->GetDevice()->CreateInputLayout(ied, 3, VS->GetBufferPointer(), VS->GetBufferSize(), &_rtlayout))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D input layout!");
	}
}

void ShadowRenderer::ConfigureRendering() {
	_d3d->SetBuffers(); // Set default Vertex, Index and Constant buffer

	// select which primtive type we are using
	_d3d->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// select Rasterizer and Sampler configuration
	_d3d->GetDeviceContext()->RSSetState(_d3d->GetRState());
	_d3d->GetDeviceContext()->PSSetSamplers(0, 1, _d3d->GetSStateWrapAddr());
	_d3d->GetDeviceContext()->PSSetSamplers(1, 1, _d3d->GetSStateClampAddr());

	// set render texture as a render target
	SetRenderTargetRenderTexture();
	SetRenderTextureShaders();

	// clear the render texture to black
	FLOAT bgColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
	_d3d->GetDeviceContext()->ClearRenderTargetView(GetRenderTexture(), bgColor);
	// clear depth buffer of render texture
	_d3d->GetDeviceContext()->ClearDepthStencilView(GetRTZBuffer(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}

void ShadowRenderer::ConfigureRenderingDebug() {
	_d3d->SetBuffers(); // Set default Vertex, Index and Constant buffer

	// select which primtive type we are using
	_d3d->GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// select Rasterizer and Sampler configuration
	_d3d->GetDeviceContext()->RSSetState(_d3d->GetRState());
	_d3d->GetDeviceContext()->PSSetSamplers(0, 1, _d3d->GetSStateWrapAddr());
	_d3d->GetDeviceContext()->PSSetSamplers(1, 1, _d3d->GetSStateClampAddr());

	// set backbuffer as a rendertarget
	_d3d->SetRenderTargetBackBuffer();
	SetRenderTextureShaders();

	// clear the render texture to deep blue
	FLOAT bgColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	_d3d->GetDeviceContext()->ClearRenderTargetView(_d3d->GetBackBuffer(), bgColor);
	// clear depth buffer of back buffer
	_d3d->GetDeviceContext()->ClearDepthStencilView(_d3d->GetZBuffer(), D3D11_CLEAR_DEPTH, 1.0f, 0);
}
