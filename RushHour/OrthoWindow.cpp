#include "stdafx.h"
#include <d3d11.h>
#include <directxmath.h>
#include <numeric>
#include "OrthoWindow.h"
#include "CommonException.h"


using namespace DirectX;

// It creates a simple 2D rectangle object for 2D orthogonal rendering
OrthoWindow::OrthoWindow(D3D* d3d, FLOAT windowWidth, FLOAT windowHeight) : _d3d(d3d), _windowWidth(windowWidth), _windowHeight(windowHeight) {
	CreateOrthoWindowModel();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateConstantBuffer();
	// Get Orthomatrix for projection
	_orthoMatrix = XMMatrixOrthographicLH((float)_windowWidth, (float)_windowHeight, 0.1f, 1000.0f);
}


OrthoWindow::~OrthoWindow() {
	if (_vBuffer) _vBuffer->Release();
	if (_iBuffer) _iBuffer->Release();
	if (_cBuffer) _cBuffer->Release();
}

DirectX::XMMATRIX OrthoWindow::GetViewMatrix() const {
	return XMMatrixLookAtLH(
		CAMINITPOSITION,
		XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f),  // the look-at position
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)   // the up direction
	);

}

void OrthoWindow::CreateOrthoWindowModel() {
	FLOAT left, right, top, bottom;

	// prepare coordinates
	left = (_windowWidth / 2) * -1;
	right = left + _windowWidth;
	top = _windowHeight / 2;
	bottom = top - _windowHeight;

	// Load the vertex vector with data.
	_objectVertices.reserve(6);
	VERTEX v;
	v.normal = { 0.0f, 0.0f, 0.0f };

	// First triangle.
	v.pos = { left, top, 0.0f };
	v.textCoord = { 0.0f, 0.0f };
	_objectVertices.push_back(v);

	v.pos = { right, bottom, 0.0f };
	v.textCoord = { 1.0f, 1.0f };
	_objectVertices.push_back(v);

	v.pos = { left, bottom, 0.0f };
	v.textCoord = { 0.0f, 1.0f };
	_objectVertices.push_back(v);

	// Second triangle.
//	v.pos = { left, top, 0.0f };
//	v.textCoord = { 0.0f, 0.0f };
//	_objectVertices.push_back(v);

	v.pos = { right, top, 0.0f };
	v.textCoord = { 1.0f, 0.0f };
	_objectVertices.push_back(v);

//	v.pos = { right, bottom, 0.0f };
//	v.textCoord = { 1.0f, 1.0f };
//	_objectVertices.push_back(v);

	// Load the index vector with data
	_objectIndices.insert(_objectIndices.end(), { 2, 1, 0, 1, 3, 0 });
}

void OrthoWindow::CreateConstantBuffer() {
	// constant buffer
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(OrthoWindow::CBUFFER);
	bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	if (FAILED(_d3d->GetDevice()->CreateBuffer(&bd, NULL, &_cBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create OrthoWindow constant buffer!");
	}
}

void OrthoWindow::CreateVertexBuffer() {
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(VERTEX) * _objectVertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	D3D11_SUBRESOURCE_DATA vertexData;
	vertexData.pSysMem = _objectVertices.data();
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	if (FAILED(_d3d->GetDevice()->CreateBuffer(&bd, &vertexData, &_vBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create OrthoWindow vertex buffer!");
	}
}

void OrthoWindow::CreateIndexBuffer() {
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DEFAULT;
	bd.ByteWidth = sizeof(UINT) * _objectIndices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;

	D3D11_SUBRESOURCE_DATA indexData;
	indexData.pSysMem = _objectIndices.data();
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	if (FAILED(_d3d->GetDevice()->CreateBuffer(&bd, &indexData, &_iBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create OrthoWindow index buffer!");
	}
}

void OrthoWindow::SetBuffers() {
	// select which vertex buffer to display
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	_d3d->GetDeviceContext()->IASetVertexBuffers(0, 1, GetVBufferAddr(), &stride, &offset);
	_d3d->GetDeviceContext()->IASetIndexBuffer(GetIBuffer(), DXGI_FORMAT_R32_UINT, 0);
	_d3d->GetDeviceContext()->VSSetConstantBuffers(0, 1, &_cBuffer);
	_d3d->SetZBufferOff();
}

void OrthoWindow::SetViewport() {
	// set the viewport
	D3D11_VIEWPORT viewport;
	ZeroMemory(&viewport, sizeof(D3D11_VIEWPORT));

	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = _windowWidth;
	viewport.Height = _windowHeight;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;

	_d3d->GetDeviceContext()->RSSetViewports(1, &viewport);
}

