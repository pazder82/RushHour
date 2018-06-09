#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include "RushHour.h"
#include "D3DSupplementary.h"
#include "D3D.h"


class TextureRenderer {
	public:
		TextureRenderer() = delete;
		TextureRenderer(D3D* d3d, FLOAT textureWidth, FLOAT textureHeight);
		TextureRenderer(D3D* d3d) : TextureRenderer(d3d, SCREEN_WIDTH, SCREEN_HEIGHT) {}
		~TextureRenderer();

		ID3D11RenderTargetView* GetRenderTexture() const { return _rTexture; }
		ID3D11ShaderResourceView** GetRenderTextureSRVAddr() { return &(_rTextureSRV); }
		ID3D11DepthStencilView* GetRTZBuffer() const { return _rtzBuffer; }
		void SetRenderTargetRenderTexture() { _d3d->SetRenderTarget(&(_rTexture), _rtzBuffer); }
		void SetRenderTextureShaders();
		virtual void LoadRenderTextureShaders() = 0;
		virtual void ConfigureRendering() = 0;

	protected:
		D3D * _d3d;
		ID3D11RenderTargetView* _rTexture;      // render texture
		ID3D11ShaderResourceView* _rTextureSRV; // render texture shader resource view
		ID3D11DepthStencilView* _rtzBuffer;     // render texture depth buffer
		ID3D11VertexShader* _rtvs;              // render texture vertex shader
		ID3D11PixelShader* _rtps;               // render texture pixel shader
		ID3D11InputLayout* _rtlayout;           // render texture layout
		FLOAT _textureWidth, _textureHeight;

		void CreateRenderTextureDepthBuffer();
		void CreateRenderTexture();
};

