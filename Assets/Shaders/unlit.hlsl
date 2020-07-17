struct VSInput {
    float4 pos : POSITION;
    float3 normal : NORMAL;
    float3 color : COLOR;
    float2 uv0 : TEXCOORD;
};

struct PSInput {
    float4 pos : SV_Position;
    float4 posWS : POSITION;
    float3 normal : NORMAL;
    float3 normalWS : NORMAL1;
    float3 color : COLOR;
    float2 uv0 : TEXCOORD;
};

struct UBO {
    row_major matrix model;
    row_major matrix view;
    row_major matrix proj;
    float3 camera_pos;
    float padding0;
};

[[vk::binding(0,0)]] ConstantBuffer<UBO> ubo;

PSInput mainVS(VSInput input) {
    PSInput output;

    output.posWS = mul(input.pos, ubo.model);
    float4 posVS = mul(output.posWS, ubo.view);
    output.pos = mul(posVS, ubo.proj);

    output.normal = input.normal;
    output.normalWS = mul( ubo.model, float4( input.normal, 0.0 ) ).xyz;
    
    
    output.uv0 = input.uv0;
    output.color = input.color;
    return output;
}

[[vk::binding(1)]] TextureCube<float4> texture_env;
[[vk::binding(1)]] SamplerState sampler_env;

[[vk::binding(2)]] Texture2D<float4> texture_albedo;
[[vk::binding(2)]] SamplerState sampler_albedo;

[[vk::binding(3)]] Texture2D<float4> texture_normal;
[[vk::binding(3)]] SamplerState sampler_normal;


// TODO: Very inefficent
float3 perturbNormal(PSInput input) {
	float3 tangentNormal = texture_normal.Sample(sampler_normal, input.uv0.xy).rgb;
    tangentNormal = normalize(tangentNormal * 2.0 - 1.0);

	float3 q1 = ddx(input.posWS.xyz);
	float3 q2 = ddy(input.posWS.xyz);
	float2 st1 = ddx(input.uv0);
	float2 st2 = ddy(input.uv0);

	float3 N = normalize(input.normalWS);
	float3 T = normalize(q1 * st2.y - q2 * st1.y);
	float3 B = -normalize(cross(N, T));
	float3x3 TBN = float3x3(T, B, N);

	return normalize(mul(TBN, tangentNormal));
}

float3x3 CotangentFrame(float3 N, float3 p, float2 uv)
{
    // get edge vectors of the pixel triangle
    float3 dp1 = ddx( p );
    float3 dp2 = ddy( p );
    float2 duv1 = ddx( uv );
    float2 duv2 = ddy( uv );
 
    // solve the linear system
    float3 dp2perp = cross( dp2, N );
    float3 dp1perp = cross( N, dp1 );
    float3 T = dp2perp * duv1.x + dp1perp * duv2.x;
    float3 B = dp2perp * duv1.y + dp1perp * duv2.y;
 
    // construct a scale-invariant frame 
    float invmax = rsqrt( max( dot(T,T), dot(B,B) ) );
    return float3x3(T * invmax,B * invmax, N);
}

float3 UnpackNormal(float4 n) 
{
    n.xyz = n.xyz * 2.0 - 1.0;
    return n.xyz;
}

float3 PerturbNormal(float3 N, float3 V, float2 uv)
{
    // assume N, the interpolated vertex normal and 
    // V, the view vector (vertex to eye)
    float3 map = UnpackNormal(texture_normal.Sample(sampler_normal, uv.xy));
    //map.y = -map.y;
    //map = map * 255./127. - 128./127.;
    float3x3 TBN = CotangentFrame(N, -V, uv);
    return normalize(mul(map, TBN));
}


float4 mainPS(PSInput input) : SV_TARGET {
    float3 light_color = float3(1.0, 1.0, 1.0);
    float3 light_pos = float3(20.0, 0.0, 10.0);

    float3 N = normalize(input.normalWS.xyz * .5 + .5);
    float3 V = normalize(input.posWS.xyz - ubo.camera_pos); 
    float3 PN = PerturbNormal(N, V, input.uv0.xy);
    float3 R = reflect(V, PN);

    float ambient_strength = 0.1;
    float3 ambient = ambient_strength * light_color;

    float3 light_dir = normalize(light_pos - input.posWS.xyz);

    float diff = max(dot(PN, light_dir), 0.0);
    float3 diffuse = diff * light_color;
    
    float3 final = texture_albedo.Sample(sampler_albedo, input.uv0.xy).rgb;

    float3 finalREF = final + texture_env.Sample(sampler_env, R).rgb ;
    float3 col = float3(1.0, 1.0, 1.0) - exp(-finalREF.rgb * 5) ;

    float3 Out_Color = float3(1.0,1.0,1.0);
    Out_Color.rgb = PN.rgb;
    //Out_Color.rgb = N.rgb;

    return float4(Out_Color, 1.0);
}