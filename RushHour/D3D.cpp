#include "stdafx.h"
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include "CommonException.h"
#include "D3D.h"

using namespace DirectX;

// this method initializes and prepares Direct3D for use
D3D::D3D(HWND hWnd) {
	CreateDevice(hWnd);
	// Create backbuffer and its zbuffer
	CreateDepthBuffer();
	SetZBufferOn();
	CreateRenderTarget();
	SetViewport();
	LoadBackBufferShaders();
	CreateConstantBuffer();
	InitRasterizer();
	InitSampler();

	_matPerspective = XMMatrixPerspectiveFovLH((FLOAT)XMConvertToRadians(45), (FLOAT)SCREEN_WIDTH / (FLOAT)SCREEN_HEIGHT, 1.0f, 100.0f);
}


D3D::~D3D() {
    // switch to windowed mode
	if (_swapChain) _swapChain->SetFullscreenState(FALSE, NULL);

	// release all the stuff
	if (_zBuffer) _zBuffer->Release();
	if (_depthDisabledStencilState) _depthDisabledStencilState->Release();
	if (_depthStencilState) _depthStencilState->Release();
	if (_layout) _layout->Release();
	if (_vs) _vs->Release();
	if (_ps) _ps->Release();
	if (_vBuffer) _vBuffer->Release();
	if (_iBuffer) _iBuffer->Release();
	if (_cBuffer) _cBuffer->Release();
	if (_swapChain) _swapChain->Release();
	if (_bBuffer) _bBuffer->Release();
	if (_rs) _rs->Release();
	if (_ssw) _ssw->Release();
	if (_ssc) _ssc->Release();
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
	scd.BufferDesc.RefreshRate.Numerator = 0;				// refresh rate: 0 -> do not care
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
	if (FAILED(_dev->CreateDepthStencilView(pDepthBuffer, &dsvd, &(_zBuffer)))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D depth buffer!");
	}
	pDepthBuffer->Release();

	// create depth buffer states
	// Initialize the description of the stencil state.
	D3D11_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Set up the description of the stencil state.
	depthStencilDesc.DepthEnable = true;
	depthStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilDesc.StencilEnable = true;
	depthStencilDesc.StencilReadMask = 0xFF;
	depthStencilDesc.StencilWriteMask = 0xFF;
	depthStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the depth stencil state.
	if (FAILED(_dev->CreateDepthStencilState(&depthStencilDesc, &_depthStencilState))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D depth buffer state!");
	}

	D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;
	ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

	// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
	// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
	depthDisabledStencilDesc.DepthEnable = false;
	depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
	depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
	depthDisabledStencilDesc.StencilEnable = false;
	depthDisabledStencilDesc.StencilReadMask = 0xFF;
	depthDisabledStencilDesc.StencilWriteMask = 0xFF;
	depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
	depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
	depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
	depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

	// Create the state using the device.
	if (FAILED(_dev->CreateDepthStencilState(&depthDisabledStencilDesc, &_depthDisabledStencilState))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D depth buffer state!");
	}

}

void D3D::CreateRenderTarget() {
	// create backbuffer and render target
	ID3D11Texture2D *pBackBuffer;
	if (FAILED(_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), reinterpret_cast<LPVOID*>(&pBackBuffer)))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to get Direct3D back buffer!");
	}
	if (FAILED(_dev->CreateRenderTargetView(pBackBuffer, NULL, &_bBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create Direct3D target view!");
	}
	pBackBuffer->Release();
}

void D3D::SetRenderTarget(ID3D11RenderTargetView** rtv, ID3D11DepthStencilView* dsv) {
	_devCon->OMSetRenderTargets(1, rtv, dsv);
}

void D3D::SetBuffers() {
	// select which vertex buffer to display
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	_devCon->IASetVertexBuffers(0, 1, GetVBufferAddr(), &stride, &offset);
	_devCon->IASetIndexBuffer(GetIBuffer(), DXGI_FORMAT_R32_UINT, 0);
	_devCon->VSSetConstantBuffers(0, 1, &_cBuffer);
	_devCon->PSSetConstantBuffers(0, 1, &_cBuffer);
	SetZBufferOn();
}

void D3D::ConfigureRenderering() {
	SetBuffers(); // Set default Vertex, Index and Constant buffer

	// select which primtive type we are using
	GetDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// select Rasterizer and Sampler configuration
	GetDeviceContext()->RSSetState(GetRState());
	GetDeviceContext()->PSSetSamplers(0, 1, GetSStateWrapAddr());
	GetDeviceContext()->PSSetSamplers(1, 1, GetSStateClampAddr());

	// set backbuffer as a rendertarget
	SetRenderTargetBackBuffer();
	SetBackBufferShaders();					

	// clear the render texture to deep blue
	FLOAT bgColor[4] = { 0.0f, 0.2f, 0.4f, 1.0f };
	GetDeviceContext()->ClearRenderTargetView(GetBackBuffer(), bgColor);
	// clear depth buffer of back buffer
	GetDeviceContext()->ClearDepthStencilView(GetZBuffer(), D3D11_CLEAR_DEPTH, 1.0f, 0);

}

void D3D::SetViewport() {
	// set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	_devCon->RSSetViewports(1, &viewport);
}

void D3D::LoadBackBufferShaders() {
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
}

void D3D::SetBackBufferShaders() {
	_devCon->IASetInputLayout(_layout);
	_devCon->VSSetShader(_vs, 0, 0);
	_devCon->PSSetShader(_ps, 0, 0);
}

void D3D::SetZBufferOn() {
	_devCon->OMSetDepthStencilState(_depthStencilState, 1);

}

void D3D::SetZBufferOff() {
	_devCon->OMSetDepthStencilState(_depthDisabledStencilState, 1);
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
	rd.AntialiasedLineEnable = TRUE;
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
	if (FAILED(_dev->CreateSamplerState(&sd, &_ssw))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to creat Direct3D wrap sampler state!");
	}
	sd.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	sd.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	if (FAILED(_dev->CreateSamplerState(&sd, &_ssc))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to creat Direct3D clamp sampler state!");
	}
}


