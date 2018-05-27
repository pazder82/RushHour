#pragma once
#include "TextureRenderer.h"
#include "OrthoWindow.h"

class OrthoWindowRenderer : public TextureRenderer {
public:
	OrthoWindowRenderer() = delete;
	OrthoWindowRenderer(D3D* d3d, OrthoWindow* orthoWindow) : TextureRenderer(d3d), _orthoWindow(orthoWindow) { LoadRenderTextureShaders(); };
	~OrthoWindowRenderer() {};

	void LoadRenderTextureShaders() override;
	void ConfigureRendering() override;

private:
	OrthoWindow* _orthoWindow;
};

