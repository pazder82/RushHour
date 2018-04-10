cbuffer ConstantBuffer {
	matrix world;
	matrix mvp;
	matrix lightmvp;
	matrix trinvworld;
	float4 camposition; // position of the camera
	float4 lightpos;    // the diffuse light's vector
	float4 diffusecol;  // the diffuse light's color
	float4 ambientcol;  // the ambient light's color
	float specularpow;  // the specular light's power
	float4 specularcol; // the specular light's color
}

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewdirection : TEXCOORD1;
	float3 lightvec : TEXCOORD2;
	float4 lightviewposition : TEXCOORD3;
};

/* VERTEX SHADER */
PixelInputType VShader(float4 position : POSITION, float3 normal : NORMAL, float2 texcoord : TEXCOORD) {
	PixelInputType output;

	float3 worldposition = mul((float3x3)world, position.xyz);

	output.position = mul(mvp, position);
	output.texcoord = texcoord;
	output.normal = normalize(mul((float3x3)trinvworld, normal));
	output.viewdirection = -(normalize(camposition.xyz - worldposition));
	output.lightvec = normalize(lightpos.xyz - worldposition);
	output.lightviewposition = mul(lightmvp, position);

	return output;
}

/* PIXEL SHADER */
Texture2D modelTexture : register(t0);
Texture2D depthMapTexture : register(t1); // texture for shadow projection
SamplerState sampleTypeWrap : register(s0);
SamplerState sampleTypeClamp : register(s1);

float4 PShader(PixelInputType input) : SV_TARGET
{
    // Set the bias value for fixing the floating point precision issues.
    float bias = 0.001f;

	// AMBIENT LIGHT
	float4 color = ambientcol;

	// DIFFUSE LIGHT
	float lightintensity = saturate(dot(input.normal, input.lightvec));
	color += diffusecol * lightintensity;
	color = saturate(color);

	// SPECULAR LIGHT
	float3 reflectedlight = reflect(input.lightvec, input.normal);
	float specularfactor = saturate(dot(reflectedlight, input.viewdirection));
	float dampedfactor = pow(specularfactor, specularpow);
	float4 specular = dampedfactor * specularcol;

	// COMBINE ALL LIGHTS AND TEXTURE TOGETHER
	color *= modelTexture.Sample(sampleTypeWrap, input.texcoord);
	color = saturate(color + specular);

	return color;
}