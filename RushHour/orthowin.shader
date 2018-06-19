cbuffer ConstantBuffer {
	matrix mvp;
	float screenWidth;
	float screenHeight;
}

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
};

/* VERTEX SHADER */
PixelInputType VShader(float4 position : POSITION, float3 normal : NORMAL, float2 texCoord : TEXCOORD) {
	PixelInputType output;

	position.w = 1.0f;
	output.position = mul(mvp, position);
	output.texCoord = texCoord;

	return output;
}

/* PIXEL SHADER */
Texture2D modelTexture : register(t0);
SamplerState sampleTypeWrap : register(s0);
SamplerState sampleTypeClamp : register(s1); // sampler for shadow projection

float4 PShader(PixelInputType input) : SV_TARGET
{
	float4 textureColor;

    // Sample the pixel color from the texture using the sampler at this texture coordinate location.
    textureColor = modelTexture.Sample(sampleTypeClamp, input.texCoord);

    return textureColor;
}