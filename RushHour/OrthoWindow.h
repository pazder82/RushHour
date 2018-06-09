#pragma once
// This class is used to perform orthographic rendering to 2D

#include "D3D.h"
#include "D3DSupplementary.h"

class OrthoWindow {
public:
	OrthoWindow() = delete;
	OrthoWindow(D3D* d3d, FLOAT windowWidth, FLOAT windowHeight);
	~OrthoWindow();

	void SetBuffers();
	void SetViewport();

	ID3D11Buffer* GetVBuffer() const { return _vBuffer; }
	ID3D11Buffer** GetVBufferAddr() { return &(_vBuffer); }
	ID3D11Buffer* GetIBuffer() const { return _iBuffer; }
	ID3D11Buffer* GetCBuffer() const { return _cBuffer; }
	FLOAT GetWindowWidth() const { return _windowWidth; }
	FLOAT GetWindowHeight() const { return _windowHeight; }
	DirectX::XMMATRIX GetOrthoMatrix() const { return _orthoMatrix; }
	DirectX::XMMATRIX GetViewMatrix() const;
	size_t GetNumOfIndices() const { return _objectIndices.size(); }

	struct CBUFFER {
		DirectX::XMMATRIX mvp;
		float screenWidth;
		float screenHeight;
	};

private:
	D3D* _d3d;
	FLOAT _windowWidth, _windowHeight;
	DirectX::XMMATRIX _orthoMatrix;

	std::vector<VERTEX> _objectVertices;
	std::vector<UINT> _objectIndices;

	ID3D11Buffer* _vBuffer;                 // vertex buffer                                      
	ID3D11Buffer* _iBuffer;                 // index buffer
	ID3D11Buffer* _cBuffer;                 // constant buffer

	void CreateOrthoWindowModel();
	void CreateVertexBuffer();
	void CreateIndexBuffer();
	void CreateConstantBuffer();
};

