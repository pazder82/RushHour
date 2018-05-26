#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include "TextureRenderer.h"
#include "RushHour.h"
#include "D3DSupplementary.h"
#include "D3D.h"


class ShadowRenderer : public TextureRenderer {
public:
	ShadowRenderer() = delete;
	ShadowRenderer(D3D* d3d) : TextureRenderer(d3d) { LoadRenderTextureShaders(); };
	~ShadowRenderer() {};

	void LoadRenderTextureShaders() override;
	void ConfigureRendering() override;
	void ConfigureRenderingDebug();

};

