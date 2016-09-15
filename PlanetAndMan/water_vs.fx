cbuffer VSConstantBuffer : register(b0)
{
	matrix View;
	matrix Projection;
	float4 Light;
}
cbuffer ConstantBuffer : register(b1)
{
	float4 PlanetPos;
}

struct VS_INPUT
{
	float3 Pos : POSITION;
	uint id : SV_VertexID;
};

struct VS_OUTPUT
{
	float4 Pos : SV_Position;
	float4 Color : COLOR;
	float2 Tex : TEXCOORD;
};

VS_OUTPUT VS(VS_INPUT input)
{
	VS_OUTPUT output = (VS_OUTPUT)0;
	output.Color.rgb = max(Light.w,
		dot(normalize(Light.xyz), normalize(input.Pos)));
	output.Color.a = 1.0f;
	output.Pos = mul(float4(input.Pos + PlanetPos.xyz, 1.0f), View);
	output.Pos = mul(output.Pos, Projection);
	//switch (input.id % 3) {
	//case 0: output.Tex = float2(4.0f, 8.f*(1.f - 0.86602540378f) + PlanetPos.w); break;
	//case 1: output.Tex = float2(8.f, 8.f + PlanetPos.w); break;
	//case 2: output.Tex = float2(0.f, 8.f + PlanetPos.w); break;
	//}
	output.Tex = float2(0.f, 0.f);
	return output;
}
