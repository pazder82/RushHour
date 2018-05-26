cbuffer ConstantBuffer {
	matrix world;
	matrix mvp;
	matrix lightMvp;
	matrix trInvWorld;
	float4 camPosition; // position of the camera
	float4 lightPos;    // the diffuse light's vector
	float4 diffuseCol;  // the diffuse light's color
	float4 ambientCol;  // the ambient light's color
	float specularPow;  // the specular light's power
	float4 specularCol; // the specular light's color
	float downsampledScreenWidth; 
	float downsampledScreenHeight;
}

struct PixelInputType {
	float4 position : SV_POSITION;
	float4 depthPosition : TEXTURE0;
};

/* VERTEX SHADER */
PixelInputType VShader(float4 position : POSITION) {
	PixelInputType output;
	position.w = 1.0f;

	// Calculate the position of the vertex against the world, view, and projection matrices.
	float4 pos = mul(lightMvp, position);
	output.position = pos;

	// Store the position value in a second input value for depth value calculations.
	output.depthPosition = pos;
	return output;
}

/* PIXEL SHADER */
float4 PShader(PixelInputType input) : SV_TARGET
{
	float depthValue;
    float4 color;

    // Get the depth value of the pixel by dividing the Z pixel depth by the homogeneous W coordinate.
    depthValue = input.depthPosition.z / input.depthPosition.w;
    color = float4(depthValue, depthValue, depthValue, 1.0f);
    return color;
}