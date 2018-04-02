cbuffer ConstantBuffer {
	matrix world;
	matrix mvp;
	matrix trinvworld;
	float4 camposition; // position of the camera
	float4 lightpos; // the diffuse light's vector
	float4 diffusecol; // the diffuse light's color
	float4 ambientcol; // the ambient light's color
	float specularpow; // the specular light's power
	float4 specularcol; // the specular light's color
}

Texture2D Texture;
SamplerState ss;

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewdirection : TEXCOORD1;
	float3 lightvec : TEXCOORD2;
};

PixelInputType VShader(float4 position : POSITION, float3 normal : NORMAL, float2 texcoord : TEXCOORD) {
	PixelInputType output;

	float3 worldposition = mul((float3x3)world, position.xyz);

	output.position = mul(mvp, position);
	output.texcoord = texcoord;
	output.normal = normalize(mul((float3x3)trinvworld, normal));
	output.viewdirection = -(normalize(camposition.xyz - worldposition));
	output.lightvec = normalize(lightpos.xyz - worldposition);

	return output;
}

float4 PShader(PixelInputType input) : SV_TARGET
{
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
	color *= Texture.Sample(ss, input.texcoord);
	color = saturate(color + specular);

	return color;
}