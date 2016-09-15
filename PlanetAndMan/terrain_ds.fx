#define ABS_ROUND_EPS 0.1f
#define EPS 0.00001f
#define PI 3.14159265f

cbuffer ConstantBuffer : register(b0)
{
	matrix View;
	matrix Projection;
	float4 Light;
}
cbuffer ConstantBuffer : register(b1)
{
	float4 PlanetPos;
}

/*
			Perlin terrain params
	x - coefficient,
	y - starting octave,
	z - number of octaves,
	w - shift
			Thresholds
	x - value,
	y - deviation,
*/
cbuffer ConstantBuffer : register(b2) 
{	
	float4 PContinents; //water or ground?
	float4 PVallMount; //ground : valleys or mountains?		-no deriv.
	float4 PPlainHills; //valleys : plains or hills			-no deriv.
	float4 PErosRidges; //mountains : erosion or ridges		-no deriv.
	float4 PHills;
	float4 PErosion;
	float4 PRidges;
	float2 PVallMountThreshold;
	float2 PPlainHillsThreshold;
	float2 PErosRidgesThreshold;
	float2 PMountHeightThreshold;
}

Texture1D<int> perlinPermutations : register(t0);

Texture1DArray<float> quinticSmoothLT : register(t1);
SamplerState LTSampler : register(s0);

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

struct DS_OUTPUT
{
	float4 Pos : SV_Position;
	float2 Tex : TEXCOORD;
	float Height : HEIGHT;
	float Bright : BRIGHTNESS;
};

float smooth(float x) 
{
	return quinticSmoothLT.SampleLevel(LTSampler, float2(x, 0.f), 0.f);
}

float smoothDeriv(float x)
{
	return quinticSmoothLT.SampleLevel(LTSampler, float2(x, 1.f), 0.f);
}

float3 gridVec(int hash) {
	switch (hash & 0xF) {
	case 0xC:
	case 0x0: return float3(1.f, 1.f, 0.f);
	case 0xE:
	case 0x1: return float3(-1.f, 1.f, 0.f);
	case 0x2: return float3(1.f, -1.f, 0.f);
	case 0x3: return float3(-1.f, -1.f, 0.f);
	case 0x4: return float3(1.f, 0.f, 1.f);
	case 0x5: return float3(-1.f, 0.f, 1.f);
	case 0x6: return float3(1.f, 0.f, -1.f);
	case 0x7: return float3(-1.f, 0.f, -1.f);
	case 0x8: return float3(0.f, 1.f, 1.f);
	case 0xD:
	case 0x9: return float3(0.f, -1.f, 1.f);
	case 0xA: return float3(0.f, 1.f, -1.f);
	case 0xF:
	case 0xB: return float3(0.f, -1.f, -1.f);
	default: return float3(0.f, 0.f, 0.f); //never happens
	}
}

float gridGrad(int hash, float x, float y, float z) {
	switch (hash & 0xF){
	case 0xC:
	case 0x0: return  x + y;
	case 0xE:
	case 0x1: return -x + y;
	case 0x2: return  x - y;
	case 0x3: return -x - y;
	case 0x4: return  x + z;
	case 0x5: return -x + z;
	case 0x6: return  x - z;
	case 0x7: return -x - z;
	case 0x8: return  y + z;
	case 0xD:
	case 0x9: return -y + z;
	case 0xA: return  y - z;
	case 0xF:
	case 0xB: return -y - z;
	default: return 0; //never happens
	}
}

float4 perlinNoise(float3 loc) {
	float4 NH;

	int3 locInt = (int3)floor(loc);
	float x = loc.x - (float)locInt.x; float x1 = x - 1.f;
	float y = loc.y - (float)locInt.y; float y1 = y - 1.f;
	float z = loc.z - (float)locInt.z; float z1 = z - 1.f;

	locInt &= 255;

	//hash coordinates
	int A = perlinPermutations[locInt.x] + locInt.y,
		AA = perlinPermutations[A] + locInt.z,
		AAA = perlinPermutations[AA],
		AAB = perlinPermutations[AA + 1],
		AB = perlinPermutations[A + 1] + locInt.z,
		ABA = perlinPermutations[AB],
		ABB = perlinPermutations[AB + 1],
		B = perlinPermutations[locInt.x + 1] + locInt.y,
		BA = perlinPermutations[B] + locInt.z,
		BAA = perlinPermutations[BA],
		BAB = perlinPermutations[BA + 1],
		BB = perlinPermutations[B + 1] + locInt.z,
		BBA = perlinPermutations[BB],
		BBB = perlinPermutations[BB + 1];

	float sx = quinticSmoothLT.SampleLevel(LTSampler, float2(x, 0.f), 0.f);
	float sy = quinticSmoothLT.SampleLevel(LTSampler, float2(y, 0.f), 0.f);
	float sz = quinticSmoothLT.SampleLevel(LTSampler, float2(z, 0.f), 0.f);

	float g000 = gridGrad(AAA, x, y, z);
	float g100 = gridGrad(BAA, x1, y, z);
	float g010 = gridGrad(ABA, x, y1, z);
	float g110 = gridGrad(BBA, x1, y1, z);
	float g001 = gridGrad(AAB, x, y, z1);
	float g101 = gridGrad(BAB, x1, y, z1);
	float g011 = gridGrad(ABB, x, y1, z1);
	float g111 = gridGrad(BBB, x1, y1, z1);
	
	float k100 = g100 - g000;
	float k010 = g010 - g000;
	float k001 = g001 - g000;
	float k110 = g110 - g010 - g100 + g000;
	float k101 = g101 - g001 - g100 + g000;
	float k011 = g011 - g001 - g010 + g000;
	float k111 = g111 - g011 - g101 + g001 - k110;

	NH.w = g000
		+ sx*k100 + sy*k010 + sz*k001
		+ sx*sy*k110
		+ sx*sz*k101
		+ sy*sz*k011
		+ sx*sy*sz*k111;

	NH.x = quinticSmoothLT.SampleLevel(LTSampler, float2(x, 1.f), 0.f) 
		* (k100 + sy*k110 + sz*k101 + sy*sz*k111);

	NH.y = quinticSmoothLT.SampleLevel(LTSampler, float2(y, 1.f), 0.f)
		* (k010 + sx*k110 + sz*k011 + sx*sz*k111);

	NH.z = quinticSmoothLT.SampleLevel(LTSampler, float2(z, 1.f), 0.f)
		* (k001 + sx*k101 + sy*k011 + sx*sy*k111);

	return NH;
}

float perlinNoiseNoDerivs(float3 loc) {

	int3 locInt = (int3)floor(loc);
	float x = loc.x - (float)locInt.x; float x1 = x - 1.f;
	float y = loc.y - (float)locInt.y; float y1 = y - 1.f;
	float z = loc.z - (float)locInt.z; float z1 = z - 1.f;

	locInt &= 255;

	//hash coordinates
	int A = perlinPermutations[locInt.x] + locInt.y,
		AA = perlinPermutations[A] + locInt.z,
		AAA = perlinPermutations[AA],
		AAB = perlinPermutations[AA + 1],
		AB = perlinPermutations[A + 1] + locInt.z,
		ABA = perlinPermutations[AB],
		ABB = perlinPermutations[AB + 1],
		B = perlinPermutations[locInt.x + 1] + locInt.y,
		BA = perlinPermutations[B] + locInt.z,
		BAA = perlinPermutations[BA],
		BAB = perlinPermutations[BA + 1],
		BB = perlinPermutations[B + 1] + locInt.z,
		BBA = perlinPermutations[BB],
		BBB = perlinPermutations[BB + 1];

	float sx = quinticSmoothLT.SampleLevel(LTSampler, float2(x, 0.f), 0.f);
	float sy = quinticSmoothLT.SampleLevel(LTSampler, float2(y, 0.f), 0.f);
	float sz = quinticSmoothLT.SampleLevel(LTSampler, float2(z, 0.f), 0.f);

	float g000 = gridGrad(AAA, x, y, z);
	float g100 = gridGrad(BAA, x1, y, z);
	float g010 = gridGrad(ABA, x, y1, z);
	float g110 = gridGrad(BBA, x1, y1, z);
	float g001 = gridGrad(AAB, x, y, z1);
	float g101 = gridGrad(BAB, x1, y, z1);
	float g011 = gridGrad(ABB, x, y1, z1);
	float g111 = gridGrad(BBB, x1, y1, z1);

	float k100 = g100 - g000;
	float k010 = g010 - g000;
	float k001 = g001 - g000;
	float k110 = g110 - g010 - g100 + g000;
	float k101 = g101 - g001 - g100 + g000;
	float k011 = g011 - g001 - g010 + g000;
	float k111 = g111 - g011 - g101 + g001 - g110 + g010 + g100 - g000;

	return g000
		+ sx*k100 + sy*k010 + sz*k001
		+ sx*sy*k110
		+ sx*sz*k101
		+ sy*sz*k011
		+ sx*sy*sz*k111;
}

[domain("tri")]
DS_OUTPUT DS(HSCF_OUTPUT input, float3 BarycentricCoordinates : SV_DomainLocation,
	const OutputPatch<HS_OUTPUT, 3> patch)
{
	DS_OUTPUT output = (DS_OUTPUT)0;
	float3 finalVertexNorm = normalize(
		BarycentricCoordinates.x * patch[0].Pos +
		BarycentricCoordinates.y * patch[1].Pos +
		BarycentricCoordinates.z * patch[2].Pos);
	float2 finalVertexTexcoord = BarycentricCoordinates.x * patch[0].Tex +
		BarycentricCoordinates.y * patch[1].Tex +
		BarycentricCoordinates.z * patch[2].Tex;
	float finalHeight = 0;
	float3 finalDeriv = float3(0.f, 0.f, 0.f);
	float4 perlin; float octave, amplitude; int octavesN;
	///////////////////////////////////////
	amplitude = PContinents.x; 
	octave = PContinents.y; 
	octavesN = (int)PContinents.z;
	for (int i = 0; i < octavesN; i++) {
		perlin = perlinNoise((finalVertexNorm + 1.f) * octave + PContinents.w) * amplitude;
		finalHeight += perlin.w;
		finalDeriv += perlin.xyz;
		octave *= 2.f; amplitude /= 2.f;
	}
	float valleysOrMountains = 0.f;
	amplitude = PVallMount.x; 
	octave = PVallMount.y; 
	octavesN = (int)PVallMount.z;
	for (int i = 0; i < octavesN; i++) {
		valleysOrMountains += perlinNoiseNoDerivs((finalVertexNorm + 1.f) * octave + PVallMount.w) * amplitude;
		octave *= 2.f;
		amplitude /= 2.f;
	}
	valleysOrMountains = clamp((valleysOrMountains - PVallMountThreshold.x) / PVallMountThreshold.y, 0.f , 1.f);
	if (valleysOrMountains < 1.f - EPS) {
		float plainsOrHills = 0.f;
		amplitude = PPlainHills.x;  
		octave = PPlainHills.y; 
		octavesN = (int)PPlainHills.z;
		for (int i = 0; i < octavesN; i++) {
			plainsOrHills += perlinNoiseNoDerivs((finalVertexNorm + 1.f) * octave + PPlainHills.w) * amplitude;
			octave *= 2.f;
			amplitude /= 2.f;
		}
		plainsOrHills = clamp((plainsOrHills - PPlainHillsThreshold.x) / PPlainHillsThreshold.y, 0.f, 1.f);
		if (plainsOrHills > EPS) {
			amplitude = plainsOrHills * (1.f - valleysOrMountains) * PHills.x; 
			octave = PHills.y; 
			octavesN = (int)PHills.z;
			for (int i = 0; i < octavesN; i++) {
				perlin = perlinNoise((finalVertexNorm + 1.f) * octave + PHills.w) * amplitude;
				finalHeight += perlin.w;
				finalDeriv += perlin.xyz;
				octave *= 2.f;
				amplitude /= 2.f;
			}
		}
	}
	if (valleysOrMountains > EPS) {
		float heightAllowsMount = clamp((finalHeight - PMountHeightThreshold.x) / PMountHeightThreshold.y, 0.f, 1.f);
		if (heightAllowsMount > EPS) {
			float erosionOrRidges = 0.f;
			amplitude = PErosRidges.x;
			octave = PErosRidges.y; 
			octavesN = (int)PErosRidges.z;
			for (int i = 0; i < octavesN; i++) {
				erosionOrRidges += perlinNoiseNoDerivs((finalVertexNorm + 1.f) * octave + PErosRidges.w) * amplitude;
				octave *= 2.f;
				amplitude /= 2.f;
			}
			erosionOrRidges = clamp((erosionOrRidges - PErosRidgesThreshold.x) / PErosRidgesThreshold.y, 0.f, 1.f);;
			if (erosionOrRidges < 1.f - EPS) {
				amplitude = (1.f - erosionOrRidges) * heightAllowsMount * valleysOrMountains * PErosion.x;
				octave = PErosion.y;
				octavesN = (int)PErosion.z;
				for (int i = 0; i < octavesN; i++) {
					perlin = perlinNoise((finalVertexNorm + 1.f) * octave + PErosion.w) * amplitude;
					finalHeight += abs(perlin.w);
					float smooth = (abs(perlin.w) > ABS_ROUND_EPS) ? 1.f : (abs(perlin.w) / ABS_ROUND_EPS);
					finalDeriv += sign(perlin.w)*perlin.xyz*smooth;
					octave *= 2.f;
					amplitude /= 2.f;
				}
			}
			if (erosionOrRidges > EPS) {
				amplitude = erosionOrRidges * heightAllowsMount * valleysOrMountains * PRidges.x;
				octave = PRidges.y;
				octavesN = (int)PRidges.z;
				for (int i = 0; i < octavesN; i++) {
					perlin = perlinNoise((finalVertexNorm + 1.f) * octave + PRidges.w) * amplitude;
					finalHeight += 0.32 * amplitude - abs(perlin.w);
					float smooth = (abs(perlin.w) > ABS_ROUND_EPS) ? 1.f : (abs(perlin.w) / ABS_ROUND_EPS);
					finalDeriv += -sign(perlin.w)*perlin.xyz*smooth;
					octave *= 2.f;
					amplitude /= 2.f;
				}
			}
		}
	}
	///////////////////////////////////////

	output.Height = finalHeight;

	finalHeight *= 0.1f;
	finalDeriv *= 0.1f;// /(PlanetPos.w + finalHeight);

	float3 finalVertexPos = PlanetPos.xyz + finalVertexNorm*(PlanetPos.w + finalHeight);
	finalVertexNorm = normalize(finalVertexNorm - (finalDeriv - dot(finalDeriv, finalVertexNorm) * finalVertexNorm));

	output.Pos = mul(float4(finalVertexPos, 1.0f), View);
	output.Pos = mul(output.Pos, Projection);

	output.Tex = finalVertexTexcoord;

	output.Bright = max(Light.w, dot(normalize(Light.xyz), finalVertexNorm));
	return output;
}