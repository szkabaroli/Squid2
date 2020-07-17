struct VSInput {
    float2 pos : POSITION;
    float2 uv0 : TEXCOORD;
    float4 color : COLOR;
};

struct VSOutput {
    float4 pos : SV_Position;
    float2 uv0 : TEXCOORD;
    float4 color : COLOR;
};

struct UBO {
    float2 scale;
    float2 translate; 
};

/*struct UBO {
    row_major matrix projection;
};*/

[[vk::binding(0, 0)]] ConstantBuffer<UBO> ubo;

VSOutput mainVS(VSInput input) {
    VSOutput output;
    
    output.color = input.color;
    output.uv0 = input.uv0;
    // output.pos = vec4(input.pos * ubo.scale + ubo.translate, 0, 1);
   
    output.pos = float4(input.pos * ubo.scale + ubo.translate, 0.0f, 1.0f);

    return output;
}

[[vk::binding(0, 1)]] Texture2D<float4> texture0;
[[vk::binding(0, 1)]] SamplerState sampler0;

float4 mainPS(VSOutput input) : SV_TARGET {
    float4 out_color = input.color * texture0.Sample(sampler0, input.uv0.xy);
    return out_color;
}