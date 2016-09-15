struct PS_INPUT
{
	float4 Pos : SV_Position;
	float4 Color : COLOR;
};

float4 PS(PS_INPUT input) : SV_Target
{
	float4 outputColor = input.Color;
	return outputColor;
}
