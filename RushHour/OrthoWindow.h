#pragma once
// This class is used to perform orthographic rendering to 2D

#include "D3D.h"
#include "D3DSupplementary.h"

class OrthoWindow {
public:
	OrthoWindow() = delete;
	OrthoWindow(D3D* d3d, UINT windowWidth, UINT windowHeight);
	~OrthoWindow();

	void SetBuffers();

	ID3D11Buffer* GetVBuffer() const { return _vBuffer; }
	ID3D11Buffer** GetVBufferAddr() { return &(_vBuffer); }
	ID3D11Buffer* GetIBuffer() const { return _iBuffer; }

private:
	D3D* _d3d;
	UINT _windowWidth, _windowHeight;

	std::vector<VERTEX> _objectVertices;
	std::vector<UINT> _objectIndices;

	ID3D11Buffer* _vBuffer;                 // vertex buffer                                      
	ID3D11Buffer* _iBuffer;                 // index buffer

};

