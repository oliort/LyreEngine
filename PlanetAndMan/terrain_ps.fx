#define EPS 0.00001f
#define EXPECTED_TEXTURES_NUMBER 32

struct PS_INPUT
{
	float4 Pos : SV_Position;
	float2 Tex : TEXCOORD;
	float Height : HEIGHT;
	float Bright : BRIGHTNESS;
};

struct TextureLayer {
	float3 height; //min, max, lower_eps
};

Texture2DArray Textures : register(t0);
StructuredBuffer<TextureLayer> TextureLayers : register(t1);
SamplerState tex2DSampler : register(s0);

float4 PS(PS_INPUT input) : SV_Target
{
	float4 outputColor;
	uint numLayers, layerSize;
	TextureLayers.GetDimensions(numLayers, layerSize);
	uint bgrTextureIndex = numLayers;
	uint i;
	[unroll(EXPECTED_TEXTURES_NUMBER)]
	for (i = numLayers - 1; i >= 0; i--) {
		if (input.Height > TextureLayers[i].height.x &&
			input.Height < TextureLayers[i].height.y) {
				bgrTextureIndex = i;
				break;
		}
	}
	if (bgrTextureIndex == numLayers) {
		outputColor = float4(1.f, 0.f, 0.f, 1.f);
		bgrTextureIndex = 0;
	}
	else outputColor = Textures.Sample(tex2DSampler, float3(input.Tex, (float)bgrTextureIndex));
	[unroll(EXPECTED_TEXTURES_NUMBER)]
	for (i = bgrTextureIndex + 1; i < numLayers; i++) {
		if (input.Height < TextureLayers[i].height.x && 
			input.Height > TextureLayers[i].height.x - TextureLayers[i].height.z) {
			outputColor = lerp(Textures.Sample(
				tex2DSampler, float3(input.Tex.x, input.Tex.y, (float)i)),
				outputColor,
				(TextureLayers[i].height.x - input.Height) / TextureLayers[i].height.z); //(hmin - h)/eps_low
		}
	}

	//OLD : float4 outputColor = input.Color * texSurface.SampleLevel(tex2DSampler, input.Tex, 0.f);
	return outputColor*input.Bright;
}
