cbuffer ConstantBuffer {
	matrix world;
	matrix rotation;
	matrix view;
	matrix projection;
	float4 lightvec; // the diffuse light's vector
	float4 lightcol; // the diffuse light's color
	float4 ambientcol; // the ambient light's color
}

Texture2D Texture;
SamplerState ss;

struct VOut {
	float4 color : COLOR;
	float2 texcoord : TEXCOORD;
	float4 position : SV_POSITION;
};

VOut VShader(float4 position : POSITION, float4 normal : NORMAL, float2 texcoord : TEXCOORD) {
	VOut output;

	// position
	output.position = mul(world, position);

	// color
	output.color = ambientcol;
	float4 norm = normalize(mul(rotation, normal)); // rotate the normal
	float diffusebrightness = saturate(dot(norm, lightvec)); // calculate the diffuse brightness
	output.color += lightcol * diffusebrightness; // find the diffuse color and add

	// texture
	output.texcoord = texcoord;

	return output;
}

float4 PShader(float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
	return color *Texture.Sample(ss, texcoord);
}