cbuffer lightBuffer : register(b0)
{
    float3 ambientLightColor;
    float ambientLightStrength;

    float3 dynamicLightColor;
    float dynamicLightStrength;
    float3 dynamicLightPosition;

    float dynamicLightAttenuation_a;
    float3 cameraPosition;
    float dynamicLightAttenuation_b;
    float dynamicLightAttenuation_c;  
}

cbuffer specBuffer : register(b1)
{
    float dynamicSpecularPower;
    float3 dynamicSpecularColor;
    float3 lightPos;
}

struct PS_INPUT
{
    float4 inPosition : SV_POSITION; // 16 byte
    float4 inLightViewPosition : TEXCOORD1; // 16 byte
    
    float3 inNormal : NORMAL; // 12 byte
    float inFogFactor : FOG; // 4 byte
    
    int isNormalMapped : ISNORMAL; // 4 byte  
    float3 inTangent : TANGENT;     // 12 byte
    
    float3 inbiNormal : BINORMAL;   // 12 byte
    float padding1 : PADDING1;
    float3 inWorldPos : WORLD_POSITION;  // 12 byte
    float padding2 : PADDING2;
    float3 inLightPos : TEXCOORD2;          // 12 byte
    float padding3 : PADDING3;
    float2 inTexCoord : TEXCOORD; // 8 byte
    int isSpecularMapped : ISSPECULAR;
    int isGlossMapped : ISGLOSSY;
};

Texture2D objTexture[3] : TEXTURE : register(t3);     // texture set when drawing models
Texture2D depthMapTexture : TEXTURE : register(t1);

SamplerState objSamplerState : SAMPLER : register(s0);
SamplerState SampleTypeClamp : SAMPLER : register(s1);

float4 main(PS_INPUT input) : SV_TARGET
{
    float bias;
    float2 projectTexCoord;
    float depthValue;
    float lightDepthValue;
    float lightIntensity;
    float3 color;
    float4 fogColor;
    float3 bumpMap;
    float3 bumpNormal;
    float3 lightDir;
    float3 viewDirection;
    
    float3 reflection;
    float4 specular;
    float4 specularIntensity;
    float4 gloss;
    float4 glossIntensity;
    float glossFactor = 1.0f;
    
    float glossPower = 10;
    
    // Initialize the specular color.
    specular = float4(0.0f, 0.0f, 0.0f, 0.0f);
    gloss = float4(0.0f, 0.0f, 0.0f, 0.0f);
    glossIntensity = float4(1.0f, 1.0f, 1.0f, 0.0f);
    
    // Set the bias value for fixing the floating point precision issues.
    bias = 0.000001f;
    
    // Set the color of the fog to grey.
    fogColor = float4(0.8f, 0.7f, 0.7f, 1.0f);
    
    // Calculate the projected texture coordinates.
    projectTexCoord.x = input.inLightViewPosition.x / input.inLightViewPosition.w / 2.0f + 0.5f;
    projectTexCoord.y = -input.inLightViewPosition.y / input.inLightViewPosition.w / 2.0f + 0.5f;
    
    color = ambientLightColor * ambientLightStrength;
    
    // Sample the pixel in the bump map.
    bumpMap = 2.0f * objTexture[1].Sample(objSamplerState, input.inTexCoord).xyz - 1.0f;
    
    const float3 vToL = input.inLightPos - input.inWorldPos;
    const float distToL = length(vToL);
    const float dirToL = vToL / distToL;
    
    float3 lDir = lightPos - input.inWorldPos;
    lDir = normalize(lDir);
    float3 lightDirInverted = -lDir;
    
    viewDirection = normalize(cameraPosition - input.inWorldPos);

    // Calculate the normal from the data in the bump map.
    bumpNormal = (bumpMap.x * input.inTangent) + (bumpMap.y * input.inbiNormal) + (bumpMap.z * input.inNormal);
	
    // Normalize the resulting bump normal.
    bumpNormal = normalize(bumpNormal);
    
    // Determine if the projected coordinates are in the 0 to 1 range.  If so then this pixel is in the view of the light.
    if ((saturate(projectTexCoord.x) == projectTexCoord.x) && (saturate(projectTexCoord.y) == projectTexCoord.y))
    {       
        // Sample the shadow map depth value from the depth texture using the sampler at the projected texture coordinate location.
        depthValue = depthMapTexture.Sample(SampleTypeClamp, projectTexCoord).r;
        
        // Calculate the depth of the light.
        lightDepthValue = input.inLightViewPosition.z / input.inLightViewPosition.w;

        // Subtract the bias from the lightDepthValue.
        lightDepthValue = lightDepthValue - bias;

        // Compare the depth of the shadow map value and the depth of the light to determine whether to shadow or to light this pixel.
        // If the light is in front of the object then light the pixel, if not then shadow this pixel since an object (occluder) is casting a shadow on it.
        if (lightDepthValue < depthValue)
        {
            // Calculate the amount of light on this pixel.
            if (input.isNormalMapped == 1)
            {
                // Sample the pixel from the specular map texture.
                if (input.isSpecularMapped == 1)
                {
                    specularIntensity = objTexture[2].Sample(objSamplerState, input.inTexCoord);
                    //glossFactor = ((1 - specularIntensity.x) + 0.02f) * 6;

                }
                               
                lightIntensity = saturate(dot(bumpNormal, lDir));
                // Calculate the reflection vector based on the light intensity, normal vector, and light direction.
                if (lightIntensity > 0.0f)
                //reflection = normalize(2 * lightIntensity * bumpNormal - lDir);
                    reflection = reflect(lightDirInverted, bumpNormal);
            }
            else if (input.isNormalMapped == 0)
            {
                lightIntensity = saturate(dot(input.inNormal, lDir));
                // Calculate the reflection vector based on the light intensity, normal vector, and light direction.
                if (lightIntensity > 0.0f)
                //reflection = normalize(2 * lightIntensity * input.inNormal - lDir);
                    reflection = reflect(lightDirInverted, normalize(input.inNormal));
            }
                
            if (lightIntensity > 0.0f)
            {
                // Determine the amount of specular light based on the reflection vector, viewing direction, and specular power.
                specular = (float4(dynamicSpecularColor, 1.0f) * (pow(saturate(dot(reflection, viewDirection)), dynamicSpecularPower)) * input.inFogFactor);
                gloss = (float4(dynamicSpecularColor, 1.0f) * (pow(saturate(dot(reflection, viewDirection)), (dynamicSpecularPower * 0.3f))) * input.inFogFactor);
                if (input.isSpecularMapped == 1)
                {
                    specular = specular * specularIntensity;
                }      

                // Determine the final diffuse color based on the diffuse color and the amount of light intensity.
                color += (ambientLightColor * (lightIntensity * dynamicLightStrength) * dynamicLightColor);

                // Saturate the final light color.
                color = saturate(color);
            }
        }
    }
    
    float3 sampleColor = objTexture[0].Sample(objSamplerState, input.inTexCoord);
    //float3 sampleColor = input.inNormal;
    
    //color = color * sampleColor;
    
    float3 ambientLight = ambientLightColor * ambientLightStrength;
    
    float3 appliedLight = ambientLight;

    float3 vectorToLight = normalize(dynamicLightPosition - input.inWorldPos);

    float3 diffuseLightIntensity = max(dot(vectorToLight, input.inNormal), 0);
    
    float distanceToLight = distance(dynamicLightPosition, input.inWorldPos);
    
    float distanceToCamera = distance(cameraPosition, input.inWorldPos);

    float attenuationFactor = 1 / (dynamicLightAttenuation_a + dynamicLightAttenuation_b * distanceToLight + dynamicLightAttenuation_c * pow(distanceToLight, 2));

    diffuseLightIntensity *= attenuationFactor;

    float3 diffuseLight = diffuseLightIntensity * dynamicLightStrength * dynamicLightColor;

    appliedLight += diffuseLight;
    
    float3 finalColor = (input.inFogFactor * color * sampleColor + (float3) (1.0 - input.inFogFactor) * (float3) fogColor);
    finalColor = saturate(finalColor + (float3) specular);
    
    specular = saturate(specular);
    return float4(saturate(finalColor), 1.0f);
    //return float4(gloss);
}