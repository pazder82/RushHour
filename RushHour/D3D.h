#pragma once

#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include "RushHour.h"
#include "D3DSupplementary.h"

class D3D {
public:
	D3D() = delete;
	D3D(HWND hWnd);
	D3D(D3D&&) = default;
	D3D& operator= (D3D&&) = default;
	~D3D();
	// D3D must not be copied, as it would release all elements after copy
	D3D(const D3D&) = delete;
	D3D& operator=(const D3D&) = delete;

	void CreateVertexBuffer(std::vector<VERTEX> OurVertices);
	void CreateIndexBuffer(std::vector<UINT> OurIndices);

	IDXGISwapChain* GetSwapChain() const { return _swapChain; };
	IDXGIDevice* GetDXGIDevice() const { return _dxgiDevice; }
	ID3D11Device* GetDevice() const { return _dev; }
	ID3D11DeviceContext* GetDeviceContext() const { return _devCon; }
	ID3D11RenderTargetView* GetBackBuffer() const { return _bBuffer; }
	ID3D11DepthStencilView* GetZBuffer() const { return _zBuffer; }
	ID3D11Buffer* GetCBuffer() const { return _cBuffer; }
	ID3D11Buffer* GetVBuffer() const { return _vBuffer; }
	ID3D11Buffer** GetVBufferAddr() { return &(_vBuffer); }
	ID3D11Buffer* GetIBuffer() const { return _iBuffer; }
	ID3D11InputLayout* GetLayout() const { return _layout; }
	ID3D11RasterizerState* GetRState() const { return _rs;  }
	ID3D11SamplerState* GetSState() const { return _ss; }
	ID3D11SamplerState** GetSStateAddr() { return &(_ss); }

private:
	IDXGISwapChain* _swapChain;             // swap chain interface
	IDXGIDevice* _dxgiDevice;
	ID3D11Device* _dev;                     // device interface
	ID3D11DeviceContext* _devCon;           // device context
	ID3D11VertexShader* _vs;                // vertex shader
	ID3D11PixelShader* _ps;                 // pixel shader
	ID3D11InputLayout* _layout;             // layout
	ID3D11RenderTargetView* _bBuffer;       // backbuffer
	ID3D11DepthStencilView* _zBuffer;       // depth buffer
	ID3D11Buffer* _cBuffer;                 // constant buffer
	ID3D11Buffer* _vBuffer;                 // vertex buffer                                      
	ID3D11Buffer* _iBuffer;                 // index buffer

	// State objects
	ID3D11RasterizerState* _rs;            // the default rasterizer state
	ID3D11SamplerState* _ss;               // sampler state

	void CreateDevice(HWND hWnd);
	void CreateDepthBuffer();
	void CreateRenderTarget();
	void SetViewport();
	void LoadShaders();
	void CreateConstantBuffer();
	void InitRasterizer();
	void InitSampler();
};

