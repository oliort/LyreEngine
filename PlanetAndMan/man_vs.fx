cbuffer VSConstantBuffer : register(b0)
{
	matrix View;
	matrix Projection;
	matrix ManRotation;
	float4 ManTranslation;
}

struct VS_INPUT
{
	float3 Pos : POSITION;
	float3 Color : COLOR;
};

struct VS_OUTPUT
{
	float4 Pos : SV_Position;
	float4 Color : COLOR;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Color.rgb = input.Color;
	output.Pos = mul(float4(input.Pos, 0.f), transpose(ManRotation));
	output.Pos += ManTranslation;
	output.Pos = mul(float4(output.Pos.xyz, 1.f), View);
	output.Pos = mul(output.Pos, Projection);
	return output;
}
