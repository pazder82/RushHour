#pragma once
#include "TextureRenderer.h"
#include "OrthoWindow.h"

class OrthoWindowRenderer : public TextureRenderer {
public:
	OrthoWindowRenderer() = delete;
	OrthoWindowRenderer(D3D* d3d, OrthoWindow* orthoWindow);
	~OrthoWindowRenderer() {};

	void LoadRenderTextureShaders() override;
	void ConfigureRendering() override;
	void ConfigureRenderingDebug();

private:
	OrthoWindow* _orthoWindow;
};

