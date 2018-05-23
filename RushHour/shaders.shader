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
}

struct PixelInputType {
	float4 position : SV_POSITION;
	float2 texCoord : TEXCOORD0;
	float3 normal : NORMAL;
	float3 viewDirection : TEXCOORD1;
	float3 lightVec : TEXCOORD2;
	float4 lightViewPosition : TEXCOORD3;
};

/* VERTEX SHADER */
PixelInputType VShader(float4 position : POSITION, float3 normal : NORMAL, float2 texCoord : TEXCOORD) {
	PixelInputType output;

	float3 worldposition = mul((float3x3)world, position.xyz);

	output.position = mul(mvp, position);
	output.texCoord = texCoord;
	output.normal = normalize(mul((float3x3)trInvWorld, normal));
	output.viewDirection = -(normalize(camPosition.xyz - worldposition));
	output.lightVec = normalize(lightPos.xyz - worldposition);
	output.lightViewPosition = mul(lightMvp, position);

	return output;
}

/* PIXEL SHADER */
Texture2D modelTexture : register(t0);
Texture2D depthMapTexture : register(t1); // depth texture for shadow projection
Texture2D shadowTexture : register(t2);   // texture with shadow
SamplerState sampleTypeWrap : register(s0);
SamplerState sampleTypeClamp : register(s1); // sampler for shadow projection

float4 PShader(PixelInputType input) : SV_TARGET
{
    // Set the bias value for fixing the floating point precision issues.
    float bias = 0.001f;

	// AMBIENT LIGHT
	float4 color = ambientCol;
	float4 specular = float4(0.0, 0.0, 0.0, 0.0);

	// Calculate the projected texture coordinates.
	float2 projectTexCoord;
	projectTexCoord.x = input.lightViewPosition.x / input.lightViewPosition.w / 2.0f + 0.5f;
	projectTexCoord.y = -input.lightViewPosition.y / input.lightViewPosition.w / 2.0f + 0.5f;
	// Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light.
	if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y)) {
		// Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
		float depthValue = depthMapTexture.Sample(sampleTypeClamp, projectTexCoord).r;
		//depthValue = 0.92f;

		// Calculate the depth of the light.
		float lightDepthValue = input.lightViewPosition.z / input.lightViewPosition.w;

		// Subtract the bias from the lightdepthvalue.
		lightDepthValue = lightDepthValue - bias;

		// Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
		// If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
		if (lightDepthValue < depthValue) {

			// DIFFUSE LIGHT
			float lightIntensity = saturate(dot(input.normal, input.lightVec));
			color += diffuseCol * lightIntensity;
			color = saturate(color);

			// SPECULAR LIGHT
			float3 reflectedLight = reflect(input.lightVec, input.normal);
			float specularFactor = saturate(dot(reflectedLight, input.viewDirection));
			float dampedFactor = pow(specularFactor, specularPow);
			specular = dampedFactor * specularCol;
		}
	}

	// COMBINE ALL LIGHTS AND TEXTURES TOGETHER
	color *= modelTexture.Sample(sampleTypeWrap, input.texCoord);
	color = saturate(color + specular);

	return color;
}