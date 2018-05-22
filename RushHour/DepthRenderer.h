#pragma once
#include <d3d11.h>
#include <directxmath.h>
#include <vector>
#include "TextureRenderer.h"
#include "RushHour.h"
#include "D3DSupplementary.h"
#include "D3D.h"


class DepthRenderer : public TextureRenderer {
public:
	DepthRenderer() = delete;
	DepthRenderer(D3D* d3d) : TextureRenderer(d3d) { LoadRenderTextureShaders(); };
	~DepthRenderer() {};

	void LoadRenderTextureShaders() override;

};

