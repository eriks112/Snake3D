#pragma pack_matrix( row_major )

cbuffer perObjectBuffer : register(b0)
{
    float4x4 viewMatrix;
    float4x4 projectionMatrix;
    float4x4 worldMatrix;
    int isNormalMapped;
    int isSpecularMapped;
    int isGlossMapped;
}; 

cbuffer lightBuffer : register(b1)
{
    float4x4 lightViewMatrix;
    float4x4 lightProjectionMatrix;
    float3 lightPosition;
}

cbuffer FogBuffer : register(b2)
{
    float fogStart;
    float fogEnd;
};

cbuffer CameraBuffer : register(b3)
{
    float3 inCameraPosition;
    float padding;
    float3 inCameraDir;
};

struct VS_INPUT
{
    float4 inPosition : POSITION;
    float2 inTexCoord : TEXCOORD;
    float3 inNormal : NORMAL;
    float3 inTangent : TANGENT;
    float3 inbiNormal : BINORMAL;
};

struct VS_OUTPUT
{
    float4 outPosition : SV_POSITION;
    float4 outLightViewPosition : TEXCOORD1;
    
    float3 outNormal : NORMAL;
    float outFogFactor : FOG;
    
    int isNormalMapped : ISNORMAL;   
    float3 outTangent : TANGENT;
    
    float3 outbiNormal : BINORMAL;
    float padding1 : PADDING1;
    float3 outWorldPos : WORLD_POSITION;   
    float padding2 : PADDING2;
    float3 outLightPos : TEXCOORD2;
    float padding3 : PADDING3;
    float2 outTexCoord : TEXCOORD;
    int isSpecularMapped : ISSPECULAR;
    int isGlossMapped : ISGLOSSY;
};

VS_OUTPUT main(VS_INPUT input)
{
    VS_OUTPUT output;
    float4 cameraPosition;
    
    // Determine if pixle is normal mapped
    output.isNormalMapped = isNormalMapped;
    output.isSpecularMapped = isSpecularMapped;
    output.isGlossMapped = isGlossMapped;
    
    // Change the position vector to be 4 units for proper matrix calculations.
    input.inPosition.w = 1.0f;
    
    // Calculate the position of the vertex against the world, view, and projection matrices.
    output.outPosition = mul(input.inPosition, worldMatrix);
    output.outPosition = mul(output.outPosition, viewMatrix);
    output.outPosition = mul(output.outPosition, projectionMatrix);
    
    output.outWorldPos = mul(input.inPosition, worldMatrix);
    
    // Calculate the position of the vertice as viewed by the light source.
    output.outLightViewPosition = mul(input.inPosition, worldMatrix);
    output.outLightViewPosition = mul(output.outLightViewPosition, lightViewMatrix);
    output.outLightViewPosition = mul(output.outLightViewPosition, lightProjectionMatrix);
    
    output.outTexCoord = input.inTexCoord;
    
    // Calculate the normal vector against the world matrix only and then normalize the final value.
    output.outNormal = mul(input.inNormal, (float3x3) worldMatrix);
    output.outNormal = normalize(output.outNormal);
    
    // Calculate the tangent vector against the world matrix only and then normalize the final value.
    output.outTangent = mul(input.inTangent, (float3x3) worldMatrix);
    output.outTangent = normalize(output.outTangent);

    // Calculate the binormal vector against the world matrix only and then normalize the final value.
    output.outbiNormal = mul(input.inbiNormal, (float3x3) worldMatrix);
    output.outbiNormal = normalize(output.outbiNormal);

    // Determine the light position based on the position of the light and the position of the vertex in the world.
    output.outLightPos = lightPosition - output.outWorldPos;

    // Normalize the light position vector.
    output.outLightPos = normalize(output.outLightPos);
    
    // Calculate the camera position.
    cameraPosition = mul(input.inPosition, worldMatrix);
    cameraPosition = mul(cameraPosition, viewMatrix);
    
    // Calculate linear fog.    
    output.outFogFactor = saturate((fogEnd - cameraPosition.z) / (fogEnd - fogStart));
    
    return output;
}