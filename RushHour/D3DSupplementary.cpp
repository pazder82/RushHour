#include "stdafx.h"
#include "D3DSupplementary.h"
#include "RushHour.h"
#include "Model.h"

IDXGISwapChain *swapchain;             // swap chain interface
ID3D11Device *dev;                     // device interface
ID3D11DeviceContext *devcon;           // device context
ID3D11RenderTargetView *backbuffer;    // backbuffer
ID3D11DepthStencilView *zbuffer;       // depth buffer
ID3D11VertexShader *pVS;               // vertex shader
ID3D11PixelShader *pPS;                // pixel shader
ID3D11Buffer *pVBuffer;                // vertex buffer                                      
ID3D11InputLayout *pLayout;            // layout
ID3D11Buffer *pCBuffer;                // constant buffer
ID3D11Buffer *pIBuffer;                // index buffer
ID3D11ShaderResourceView *pTexture;    // texture buffer

// State objects
ID3D11RasterizerState *pRS;            // the default rasterizer state
ID3D11SamplerState *pSS;               // sampler state

void CreateDevice(HWND hWnd) {
	DXGI_SWAP_CHAIN_DESC scd;
	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	// fill the swap chain description struct
	scd.BufferCount = 1;                                    // one back buffer
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;     // use 32-bit color
	scd.BufferDesc.Width = SCREEN_WIDTH;                    // set the back buffer width
	scd.BufferDesc.Height = SCREEN_HEIGHT;                  // set the back buffer height
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;      // how swap chain is to be used
	scd.OutputWindow = hWnd;                                // the window to be used
	scd.SampleDesc.Count = 4;                               // how many multisamples
	scd.Windowed = RUNINWINDOW;                             // windowed/full-screen mode
	scd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;     // allow full-screen switching by Alt-Enter

    // create a device, device context and swap chain using the information in the scd struct
	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL,
		NULL, D3D11_SDK_VERSION, &scd, &swapchain, &dev, NULL, &devcon);
}

void CreateDepthBuffer() {
	// create the depth buffer texture
	D3D11_TEXTURE2D_DESC texd;
	ZeroMemory(&texd, sizeof(texd));
	texd.Width = SCREEN_WIDTH;
	texd.Height = SCREEN_HEIGHT;
	texd.ArraySize = 1;
	texd.MipLevels = 1;
	texd.SampleDesc.Count = 4;
	texd.Format = DXGI_FORMAT_D32_FLOAT;
	texd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	ID3D11Texture2D *pDepthBuffer;
	dev->CreateTexture2D(&texd, NULL, &pDepthBuffer);

	// create the depth buffer
	D3D11_DEPTH_STENCIL_VIEW_DESC dsvd;
	ZeroMemory(&dsvd, sizeof(dsvd));
	dsvd.Format = DXGI_FORMAT_D32_FLOAT;
	dsvd.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;
	dev->CreateDepthStencilView(pDepthBuffer, &dsvd, &zbuffer);
	pDepthBuffer->Release();
}

void CreateRenderTarget() {
	// create backbuffer and render target
	ID3D11Texture2D *pBackBuffer;
	swapchain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	dev->CreateRenderTargetView(pBackBuffer, NULL, &backbuffer);
	pBackBuffer->Release();
	devcon->OMSetRenderTargets(1, &backbuffer, zbuffer);
}

void SetViewport() {
	// set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = SCREEN_WIDTH;
	viewport.Height = SCREEN_HEIGHT;
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1;

	devcon->RSSetViewports(1, &viewport);
}

void LoadShaders() {
    // load and compile vertex and pixel shader
    ID3D10Blob *VS, *PS, *Errors;
    D3DX11CompileFromFile(L"shaders.shader", 0, 0, "VShader", "vs_4_0", 0, 0, 0, &VS, &Errors, 0);
	if (Errors)
		MessageBox(NULL, L"The vertex shader failed to compile.", L"Error", MB_OK);
	ZeroMemory(&Errors, sizeof(Errors));
    D3DX11CompileFromFile(L"shaders.shader", 0, 0, "PShader", "ps_4_0", 0, 0, 0, &PS, &Errors, 0);
	if (Errors)
		MessageBox(NULL, L"The pixel shader failed to compile.", L"Error", MB_OK);
    dev->CreateVertexShader(VS->GetBufferPointer(), VS->GetBufferSize(), NULL, &pVS);
    dev->CreatePixelShader(PS->GetBufferPointer(), PS->GetBufferSize(), NULL, &pPS);
    devcon->VSSetShader(pVS, 0, 0);
    devcon->PSSetShader(pPS, 0, 0);

    // create the input layout object
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };
	
    dev->CreateInputLayout(ied, 3, VS->GetBufferPointer(), VS->GetBufferSize(), &pLayout);
    devcon->IASetInputLayout(pLayout);
}

void CreateConstantBuffer() {
	// constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(CBUFFER);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	dev->CreateBuffer(&bd, NULL, &pCBuffer);
	devcon->VSSetConstantBuffers(0, 1, &pCBuffer);
}

void CreateVertexBuffer(std::vector<VERTEX> OurVertices) {
	// create the vertex buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX) * OurVertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	dev->CreateBuffer(&bd, NULL, &pVBuffer);

    // copy the vertices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	devcon->Map(pVBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
	memcpy(ms.pData, OurVertices.data(), sizeof(VERTEX) * OurVertices.size());
	devcon->Unmap(pVBuffer, NULL);
}

void CreateIndexBuffer(std::vector<UINT> OurIndices) {
    // create the index buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
    bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(UINT) * OurIndices.size();
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    bd.MiscFlags = 0;
    dev->CreateBuffer(&bd, NULL, &pIBuffer);

	// copy the indices into the buffer
	D3D11_MAPPED_SUBRESOURCE ms;
    devcon->Map(pIBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms);
    memcpy(ms.pData, OurIndices.data(), sizeof(UINT) * OurIndices.size());
    devcon->Unmap(pIBuffer, NULL);
}

void LoadTextures() {
	// read the texture
	D3DX11CreateShaderResourceViewFromFile(dev, L"t.jpg", NULL, NULL, &pTexture, NULL);
}

void InitRasterizer() {
	D3D11_RASTERIZER_DESC rd;
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_BACK;
	rd.FrontCounterClockwise = FALSE;
	rd.DepthClipEnable = TRUE;
	rd.ScissorEnable = FALSE;
	rd.AntialiasedLineEnable = FALSE;
	rd.MultisampleEnable = FALSE;
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0.0f;
	rd.SlopeScaledDepthBias = 0.0f;
	dev->CreateRasterizerState(&rd, &pRS);
}

void InitSampler() {
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
	dev->CreateSamplerState(&sd, &pSS);
}

// this function initializes and prepares Direct3D for use
void InitD3D(HWND hWnd) {
	CreateDevice(hWnd);
	CreateDepthBuffer();
	CreateRenderTarget();
	SetViewport();
	LoadShaders();
	CreateConstantBuffer();
	InitRasterizer();
	InitSampler();
}

// this is the function that cleans up Direct3D and COM
void CleanD3D() {
	swapchain->SetFullscreenState(FALSE, NULL);    // switch to windowed mode

	// release all the stuff
	zbuffer->Release();
	pLayout->Release();
	pVS->Release();
	pPS->Release();
	pVBuffer->Release();
	pIBuffer->Release();
	pCBuffer->Release();
	pTexture->Release();
	swapchain->Release();
	backbuffer->Release();
	pRS->Release();
	pSS->Release();
	dev->Release();
	devcon->Release();
}
