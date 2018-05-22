#include "stdafx.h"
#include "TextureRenderer.h"


#include "stdafx.h"
#include <d3dcompiler.h>
#include "TextureRenderer.h"
#include "CommonException.h"

TextureRenderer::TextureRenderer(D3D* d3d) : _d3d(d3d) {
	// Create shadow render texture and its zbuffer
	CreateRenderTextureDepthBuffer();
	CreateRenderTexture();
}

TextureRenderer::~TextureRenderer() {
	if (_rtzBuffer) _rtzBuffer->Release();
	if (_rTextureSRV) _rTextureSRV->Release();
	if (_rTexture) _rTexture->Release();
}

void TextureRenderer::CreateRenderTextureDepthBuffer() {
	// create the depth buffer texture
	D3D11_TEXTURE2D_DESC texd;
	ZeroMemory(&texd, sizeof(texd));
	texd.Width = SCREEN_WIDTH;
	texd.Height = SCREEN_HEIGHT;
	texd.ArraySize = 1;
	texd.MipLevels = 1;
	texd.SampleDesc.Count = 1;
	texd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texd.Usage = D3D11_USAGE_DEFAULT;
	texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ID3D11Texture2D *pDepthBuffer;
	if (FAILED(_d3d->_dev->CreateTexture2D(&texd, NULL, &pDepthBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D depth buffer texture!");
	}

	// create the depth buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(dsvd));
	//dsvd.Format = DXGI_FORMAT_D32_FLOAT;
	dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	if (FAILED(_d3d->_dev->CreateDepthStencilView(pDepthBuffer, &dsvd, &(_rtzBuffer)))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D depth buffer!");
	}
	pDepthBuffer->Release();
}

void TextureRenderer::CreateRenderTexture() {
	ID3D11Texture2D* renderTexture;
	D3D11_TEXTURE2D_DESC textureDesc;

	// Initialize the render target texture description.
	ZeroMemory(&textureDesc, sizeof(textureDesc));

	// Setup the render target texture description.
	textureDesc.Width = SCREEN_WIDTH;
	textureDesc.Height = SCREEN_HEIGHT;
	textureDesc.MipLevels = 1;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = 0;

	// Create the render target texture.
	if (FAILED(_d3d->_dev->CreateTexture2D(&textureDesc, NULL, &renderTexture))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D render texture!");
	}

	// Setup the description of the render target view.
	D3D11_RENDER_TARGET_VIEW_DESC renderTargetViewDesc;
	renderTargetViewDesc.Format = textureDesc.Format;
	renderTargetViewDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	renderTargetViewDesc.Texture2D.MipSlice = 0;

	// Create the render target view.
	if (FAILED(_d3d->_dev->CreateRenderTargetView(renderTexture, &renderTargetViewDesc, &_rTexture))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D render texture target view!");
	}

	// Setup the description of the shader resource view.
	D3D11_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc;
	shaderResourceViewDesc.Format = textureDesc.Format;
	shaderResourceViewDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
	shaderResourceViewDesc.Texture2D.MipLevels = 1;

	// Create the shader resource view.
	if (FAILED(_d3d->_dev->CreateShaderResourceView(renderTexture, &shaderResourceViewDesc, &_rTextureSRV))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D render texture shader resource view!");
	}

	renderTexture->Release();
}

void TextureRenderer::SetRenderTextureShaders() {
	_d3d->_devCon->IASetInputLayout(_rtlayout);
	_d3d->_devCon->VSSetShader(_rtvs, 0, 0);
	_d3d->_devCon->PSSetShader(_rtps, 0, 0);
}
