#ifndef __d3dsupplementary__
#define __d3dsupplementary__

#include <d3d11.h>
#include <directxmath.h>

struct VERTEX { 
	DirectX::XMFLOAT3 pos;
	DirectX::XMFLOAT3 normal;
	DirectX::XMFLOAT2 textCoord;
};
struct CBUFFER {
	DirectX::XMMATRIX world;
	DirectX::XMMATRIX mvp;
	DirectX::XMMATRIX invTrWorld;
	DirectX::XMVECTOR cameraPosition;
	DirectX::XMVECTOR lightVector;
	DirectX::XMVECTOR diffuseColor;
	DirectX::XMVECTOR ambientColor;
	float specularPower;
	DirectX::XMVECTOR specularColor;
};
#endif