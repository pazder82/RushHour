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

struct VOut {
	float4 color : COLOR;
	float2 texcoord : TEXCOORD0;
	float4 position : SV_POSITION;
};

VOut VShader(float4 position : POSITION, float3 normal : NORMAL, float2 texcoord : TEXCOORD) {
	VOut output;

	// VERTEX POSITION TRANSFORMATION
	// Calculate the position of the vertex in the world
//	float4 worldposition4 = mul(world, position);
	float3 worldposition = mul((float3x3)world, position.xyz);
	output.position = mul(mvp, position);

	// AMBIENT LIGHT & NORMAL TRANSFORMATION
	float4 color = ambientcol;
	float3 norm = normalize(mul((float3x3)trinvworld, normal));
	//float3 norm = normalize(mul((float3x3)world, normal));

	// DIFFUSE LIGHT
	float3 lightpos3 = lightpos.xyz;
	float3 lightvec = normalize(lightpos3 - worldposition);
//	float4 lightvec = normalize(lightpos - worldposition4);
//	lightvec = -lightvec;
	float lightintensity = saturate(dot(norm, lightvec)); // calculate the amount of light

	// SPECULAR LIGHT
	float3 camposition3 = camposition.xyz;
	float3 camvec = normalize(camposition3 - worldposition);
	camvec = -camvec;
	float3 reflectedlight = reflect(lightvec, norm);

	float specularfactor = saturate(dot(reflectedlight, camvec));
	float dampedfactor = pow(specularfactor, specularpow);
	float4 finalspecular = dampedfactor * specularcol;
	
	// COMBINE ALL LIGTHS TOGETHER
	color += diffusecol * lightintensity;// +finalspecular;
	output.color = saturate(color);

	// TEXTURE
	output.texcoord = texcoord;

	return output;
}

float4 PShader(float4 color : COLOR, float2 texcoord : TEXCOORD) : SV_TARGET
{
	return color *Texture.Sample(ss, texcoord);
}