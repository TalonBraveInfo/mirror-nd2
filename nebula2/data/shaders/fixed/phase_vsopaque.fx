//------------------------------------------------------------------------------
//  phase_vsopaque.fx
//
//  Prepare opaque rendering with vertex shaders under DX7.
//
//  (C) 2004 RadonLabs GmbH
//------------------------------------------------------------------------------
shared float3 LightPos;             // the light's position in world space
shared float4 LightAmbient;         // light's ambient component
shared float4 LightDiffuse;         // light's diffuse component
shared float4 LightSpecular;        // light's specular component

technique t0
{
    pass p0
    {
        ZWriteEnable     = True;
        AlphaBlendEnable = False;
        AlphaTestEnable  = True;
        AlphaFunc        = GreaterEqual;
        Lighting         = False;
        FogEnable        = False;
        TextureTransformFlags[0] = 0;
    }
}