#include "stdafx.h"
#include <d3d11.h>
#include <directxmath.h>
#include <numeric>
#include "OrthoWindow.h"
#include "CommonException.h"


using namespace DirectX;

// It creates a simple 2D rectangle object for 2D orthogonal rendering
OrthoWindow::OrthoWindow(D3D* d3d, UINT windowWidth, UINT windowHeight) : _d3d(d3d), _windowWidth(windowWidth), _windowHeight(windowHeight) {
	CreateOrthoWindowModel();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreateConstantBuffer();
	// Get Orthomatrix for projection
	_orthoMatrix = XMMatrixOrthographicLH(_windowWidth, _windowHeight, 1.0f, 100.0f);
/*
	_cBufferData.downsampledScreenHeight = _windowHeight;
	_cBufferData.downsampledScreenWidth = _windowWidth;
	_cBufferData.mvp = _orthoMatrix * ...;
	_d3d->GetDeviceContext()->UpdateSubresource(GetCBuffer(), 0, 0, &_cBufferData, 0, 0);
	*/
}


OrthoWindow::~OrthoWindow() {
	if (_vBuffer) _vBuffer->Release();
	if (_iBuffer) _iBuffer->Release();
	if (_cBuffer) _cBuffer->Release();
}

void OrthoWindow::CreateOrthoWindowModel() {
	FLOAT left, right, top, bottom;

	// prepare coordinates
	left = (float)((_windowWidth / 2) * -1);
	right = left + (float)_windowWidth;
	top = (float)(_windowHeight / 2);
	bottom = top - (float)_windowHeight;

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
	v.pos = { left, top, 0.0f };
	v.textCoord = { 0.0f, 0.0f };
	_objectVertices.push_back(v);

	v.pos = { right, top, 0.0f };
	v.textCoord = { 1.0f, 0.0f };
	_objectVertices.push_back(v);

	v.pos = { right, bottom, 0.0f };
	v.textCoord = { 1.0f, 1.0f };
	_objectVertices.push_back(v);

	// Load the index vector with data
	_objectIndices.insert(_objectIndices.end(), { 0, 1, 2, 3, 4, 5 });
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
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX) * _objectVertices.size();
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(_d3d->GetDevice()->CreateBuffer(&bd, NULL, &_vBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create OrthoWindow vertex buffer!");
	}

	// Copy the vertices into the vertex buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	if (FAILED(_d3d->GetDeviceContext()->Map(_vBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to map OrthoWindow vertex buffer!");
	}
	memcpy(ms.pData, _objectVertices.data(), sizeof(VERTEX) * _objectVertices.size());
	_d3d->GetDeviceContext()->Unmap(_vBuffer, NULL);
}

void OrthoWindow::CreateIndexBuffer() {
	D3D11_BUFFER_DESC bd;
	ZeroMemory(&bd, sizeof(bd));
	bd.Usage = D3D11_USAGE_DYNAMIC;
	bd.ByteWidth = sizeof(VERTEX) * _objectIndices.size();
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	if (FAILED(_d3d->GetDevice()->CreateBuffer(&bd, NULL, &_iBuffer))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to create OrthoWindow index buffer!");
	}

	// Copy the indices into the index buffer
	D3D11_MAPPED_SUBRESOURCE ms;
	if (FAILED(_d3d->GetDeviceContext()->Map(_iBuffer, NULL, D3D11_MAP_WRITE_DISCARD, NULL, &ms))) {
		throw CommonException((LPWSTR)L"Critical error: Unable to map OrthoWindow index buffer!");
	}
	memcpy(ms.pData, _objectIndices.data(), sizeof(UINT) * _objectIndices.size());
	_d3d->GetDeviceContext()->Unmap(_iBuffer, NULL);

}

void OrthoWindow::SetBuffers() {
	// select which vertex buffer to display
	UINT stride = sizeof(VERTEX);
	UINT offset = 0;
	_d3d->GetDeviceContext()->IASetVertexBuffers(0, 1, GetVBufferAddr(), &stride, &offset);
	_d3d->GetDeviceContext()->IASetIndexBuffer(GetIBuffer(), DXGI_FORMAT_R32_UINT, 0);
	_d3d->GetDeviceContext()->VSSetConstantBuffers(0, 1, &_cBuffer);
}

