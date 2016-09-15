cbuffer HSConstantBuffer : register(b0)
{
	float4 CameraPosAndVisibleDist;
	float MinDist;
	float MaxDist;
	float MinLOD;
	float MaxLOD;
}

struct HS_INPUT
{
	float3 Pos : CONTROL_POINT_WORLD_POSITION;
};

struct HS_OUTPUT
{
	float3 Pos : CONTROL_POINT_WORLD_POSITION;
	float2 Tex : CONTROL_POINT_TEXCOORD;
};

struct HSCF_OUTPUT
{
	float edge[3]  :  SV_TessFactor;
	float inside[1]  :  SV_InsideTessFactor;
};

static const float PI = 3.14159265f;

float ComputePatchLOD(float3 midPoint)
{
	float dist = distance(CameraPosAndVisibleDist.xyz, midPoint);
	if (dist > CameraPosAndVisibleDist.w) return 0.f;
	float d = 1.f - (clamp(dist, MinDist, MaxDist) - MinDist) / (MaxDist - MinDist);
	return lerp(MinLOD, MaxLOD, pow(2.f, 8.f * (d - 1.f)));
}

HSCF_OUTPUT HSCF(InputPatch<HS_INPUT, 6> patch, uint PatchID :  SV_PrimitiveID)
{
	HSCF_OUTPUT output = (HSCF_OUTPUT)0;
	// Determine the midpoints of this patch
	float3 midPoints[] = {
		(patch[0].Pos + patch[1].Pos + patch[2].Pos) / 3.0f,
		(patch[1].Pos + patch[3].Pos + patch[2].Pos) / 3.0f,
		(patch[2].Pos + patch[4].Pos + patch[0].Pos) / 3.0f,
		(patch[0].Pos + patch[5].Pos + patch[1].Pos) / 3.0f
	};
	float lods[] = {
		ComputePatchLOD(midPoints[0]),
		ComputePatchLOD(midPoints[1]),
		ComputePatchLOD(midPoints[2]),
		ComputePatchLOD(midPoints[3])
	};
	output.inside[0] = lods[0];
	output.edge[0] = min(lods[0], lods[1]);
	output.edge[1] = min(lods[0], lods[2]);
	output.edge[2] = min(lods[0], lods[3]);
	return output;
}

[domain("tri")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("HSCF")]
HS_OUTPUT HS(InputPatch<HS_INPUT, 6> patch, uint i : SV_OutputControlPointID)
{
	HS_OUTPUT output = (HS_OUTPUT)0;
	output.Pos = patch[i].Pos;
	switch (i) {
	case 0: output.Tex = float2(5.0f, 10.f*(1.f - 0.86602540378f)); break;
	case 1: output.Tex = float2(10.f, 10.f); break;
	case 2: output.Tex = float2(0.f, 10.f); break;
	}
	return output;
}