






struct VSInput {
    float2 pos : POSITION;
    float2 uv  : TEXCOORD0;
};

struct PSInput {
    float4 pos : SV_Position;
    float2 uv  : TEXCOORD0;
};









PSInput main(VSInput input)



{

    PSInput output;
    output.pos = float4(input.pos, 0.0, 1.0);
    output.uv = input.uv;
    return output;




}
