#ifndef __d3dsupplementary__
#define __d3dsupplementary__

#include <d3d11.h>
#include <d3dx11.h>
#include <d3dx10.h>
#include <directxmath.h>
#include <vector>

// global declarations

extern IDXGISwapChain *swapchain;             // swap chain interface
extern ID3D11Device *dev;                     // device interface
extern ID3D11DeviceContext *devcon;           // device context
extern ID3D11RenderTargetView *backbuffer;    // backbuffer
extern ID3D11DepthStencilView *zbuffer;       // depth buffer
extern ID3D11Buffer *pVBuffer;                // vertex buffer
extern ID3D11InputLayout *pLayout;            // layout

struct VERTEX { 
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 textCoord;
};
struct CBUFFER {
	D3DMATRIX world;
	DirectX::XMFLOAT4X4  rotation;
	D3DMATRIX view;
	D3DMATRIX projection;
	DirectX::XMVECTOR diffuseVector;
	DirectX::XMVECTOR diffuseColor;
	DirectX::XMVECTOR ambientColor;
};
extern ID3D11Buffer *pCBuffer;                // constant buffer
extern ID3D11Buffer *pIBuffer;                // index buffer
extern ID3D11ShaderResourceView *pTexture;    // texture buffer


void InitD3D(HWND hWnd);     // sets up and initializes Direct3D
void CleanD3D(void);         // closes Direct3D and releases memory
void LoadTextures();         // load textures
void CreateVertexBuffer(std::vector<VERTEX> OurVertices); // create vertex buffer
void CreateIndexBuffer(std::vector<UINT> OurIndices);     // create index buffer
#endif