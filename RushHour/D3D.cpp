#include "stdafx.h"
#include <d3dcompiler.h>
#include "CommonException.h"
#include "D3D.h"


D3D::D3D(HWND hWnd) {
	// this function initializes and prepares Direct3D for use
	CreateDevice(hWnd);
	CreateDepthBuffer();
	CreateRenderTarget();
	SetViewport();
	LoadShaders();
	CreateConstantBuffer();
	InitRasterizer();
	InitSampler();
}


D3D::~D3D() {
    // switch to windowed mode
	if (_swapChain) _swapChain->SetFullscreenState(FALSE, NULL);

	// release all the stuff
	if (_zBuffer) _zBuffer->Release();
	if (_layout) _layout->Release();
	if (_vs) _vs->Release();
	if (_ps) _ps->Release();
	if (_vBuffer) _vBuffer->Release();
	if (_iBuffer) _iBuffer->Release();
	if (_cBuffer) _cBuffer->Release();
	if (_swapChain) _swapChain->Release();
	if (_bBuffer) _bBuffer->Release();
	if (_rs) _rs->Release();
	if (_ss) _ss->Release();
	if (_dev) _dev->Release();
	if (_devCon) _devCon->Release();
}

void D3D::CreateDevice(HWND hWnd) {
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferDesc.Width = SCREEN_WIDTH;                    // set the back buffer width
	scd.BufferDesc.Height = SCREEN_HEIGHT;                  // set the back buffer height
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.RefreshRate.Numerator = 0;									// refresh rate: 0 -> do not care
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;     // use 32-bit color
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;		// unspecified scan line ordering
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;						// unspecified scaling
	scd.OutputWindow = hWnd;                                // the window to be used
	scd.SampleDesc.Count = 4;                               // how many multisamples
	scd.SampleDesc.Quality = 0;
	scd.Windowed = RUNINWINDOW;                             // windowed/full-screen mode
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching by Alt-Enter

	D3D_FEATURE_LEVEL featureLevel;

	// create a device, device context and swap chain using the information in the scd struct
	if (FAILED(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, 
		D3D11_CREATE_DEVICE_BGRA_SUPPORT /* This flag is necessary for compatibility with Direct2D */, 
		NULL, NULL, D3D11_SDK_VERSION, &scd, &_swapChain, &_dev, &featureLevel, &_devCon))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create the Direct3D device!");
	} else if (featureLevel < D3D_FEATURE_LEVEL_11_0) {
		throw CommonException((LPWSTR)L"Critical error: You need DirectX 11.0 or higher to run this game!");
	}

	if (FAILED(_dev->QueryInterface(__uuidof(IDXGIDevice), reinterpret_cast<LPVOID*>(&_dxgiDevice)))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to get Direct3D DXGI device!");
	}
}

void D3D::CreateDepthBuffer() {
	// create the depth buffer texture
	D3D11_TEXTURE2D_DESC texd;
	ZeroMemory(&texd, sizeof(texd));
	texd.Width = SCREEN_WIDTH;
	texd.Height = SCREEN_HEIGHT;
	texd.ArraySize = 1;
	texd.MipLevels = 1;
	texd.SampleDesc.Count = 4;
	//texd.Format = DXGI_FORMAT_D32_FLOAT;
	texd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	texd.Usage = D3D11_USAGE_DEFAULT;
	texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ID3D11Texture2D *pDepthBuffer;
	if (FAILED(_dev->CreateTexture2D(&texd, NULL, &pDepthBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D depth buffer texture!");
	}

	// create the depth buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(dsvd));
	//dsvd.Format = DXGI_FORMAT_D32_FLOAT;
	dsvd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	if (FAILED(_dev->CreateDepthStencilView(pDepthBuffer, &dsvd, &_zBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D depth buffer!");
	}
	pDepthBuffer->Release();
}

void D3D::CreateRenderTarget() {
	// create backbuffer and render target
	ID3D11Texture2D *pBackBuffer;
	if (FAILED(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer)))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to get Direct3D depth buffer!");
	}
	if (FAILED(_dev->CreateRenderTargetView(pBackBuffer, NULL, &_bBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D depth buffer!");
	}
	pBackBuffer->Release();
	_devCon->OMSetRenderTargets(1, &_bBuffer, _zBuffer);
}

void D3D::SetViewport() {
	// set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	_devCon->RSSetViewports(1, &viewport);
}

void D3D::LoadShaders() {
	// load and compile vertex and pixel shader
	ID3D10Blob *VS, *PS;
	D3DCompileFromFile(L"shaders.shader", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VShader", "vs_4_0", 0, 0, &VS, NULL);
	D3DCompileFromFile(L"shaders.shader", NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PShader", "ps_4_0", 0, 0, &PS, NULL);
	if (FAILED(_dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &_vs))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D vertex shader!");
	}
	if (FAILED(_dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &_ps))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D pixel shader!");
	}
	_devCon->VSSetShader(_vs, 0, 0);
	_devCon->PSSetShader(_ps, 0, 0);

	// create the input layout object
	D3D11_INPUT_ELEMENT_DESC ied[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	    { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	if (FAILED(_dev->CreateInputLayout(ied, 3, VS->GetBufferPointer(), VS->GetBufferSize(), &_layout))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D input layout!");
	}
	_devCon->IASetInputLayout(_layout);
}

void D3D::CreateConstantBuffer() {
	// constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBUFFER);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	if (FAILED(_dev->CreateBuffer(&bd, NULL, &_cBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D constant buffer!");
	}
	_devCon->VSSetConstantBuffers(0, 1, &_cBuffer);
}

void D3D::CreateVertexBuffer(std::vector<VERTEX> OurVertices) {
	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX) * OurVertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(_dev->CreateBuffer(&bd, NULL, &_vBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D vertex buffer!");
	}

	// copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	if (FAILED(_devCon->Map(_vBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to map Direct3D vertex buffer!");
	}
	memcpy(ms.pData, OurVertices.data(), sizeof(VERTEX) * OurVertices.size());
	_devCon->Unmap(_vBuffer, NULL);
}

void D3D::CreateIndexBuffer(std::vector<UINT> OurIndices) {
	// create the index buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(UINT) * OurIndices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	bd.MiscFlags = 0;
	if (FAILED(_dev->CreateBuffer(&bd, NULL, &_iBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D index buffer!");
	}

	// copy the indices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	if (FAILED(_devCon->Map(_iBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to map Direct3D index buffer!");
	}
	memcpy(ms.pData, OurIndices.data(), sizeof(UINT) * OurIndices.size());
	_devCon->Unmap(_iBuffer, NULL);
}

void D3D::InitRasterizer() {
	D3D11_RASTERIZER_DESC rd;
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_BACK;
	rd.FrontCounterClockwise = TRUE;
	rd.DepthClipEnable = TRUE;
	rd.ScissorEnable = FALSE;
	rd.AntialiasedLineEnable = FALSE;
	rd.MultisampleEnable = FALSE;
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0.0f;
	rd.SlopeScaledDepthBias = 0.0f;
	if (FAILED(_dev->CreateRasterizerState(&rd, &_rs))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to creat Direct3D rasterizer state!");
	}
}

void D3D::InitSampler() {
	D3D11_SAMPLER_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	sd.MaxAnisotropy = 16;
	sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sd.BorderColor[0] = 0.0f;
	sd.BorderColor[1] = 0.0f;
	sd.BorderColor[2] = 0.0f;
	sd.BorderColor[3] = 0.0f;
	sd.ComparisonFunc = D3D11_COMPARISON_NEVER;
	sd.MinLOD = 0.0f;
	sd.MaxLOD = FLT_MAX;
	sd.MipLODBias = 0.0f;
	if (FAILED(_dev->CreateSamplerState(&sd, &_ss))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to creat Direct3D sampler state!");
	}
}


