#define N_IMPLEMENTS nMRTSceneServer
//------------------------------------------------------------------------------
//  nmrtsceneserver_emissive.cc
//  (C) 2003 RadonLabs GmbH
//------------------------------------------------------------------------------
#include "scene/nmrtsceneserver.h"
#include "gfx2/ngfxserver2.h"
#include "gfx2/nshader2.h"
#include "gfx2/nmesh2.h"

//------------------------------------------------------------------------------
/**
    Transfer the contents of the source buffer to the current render target,
    and apply a convolution filter which blurs the image in the source
    buffer.
*/
void
nMRTSceneServer::ApplyBlurFilter(nGfxServer2* gfxServer, nTexture2* srcBuffer)
{
    n_assert(gfxServer);
    n_assert(srcBuffer);

    const int numSamples = 4;
    static float4 sampleOffset[numSamples] = 
    {
        { +1.5f,  0.0f, 0.0f, 0.0f },  
        { -1.5f,  0.0f, 0.0f, 0.0f },  
        {  0.0f, +1.5f, 0.0f, 0.0f }, 
        {  0.0f, -1.5f, 0.0f, 0.0f },
    };
    static float4 sampleWeight[numSamples] =
    {
        { 0.24f, 0.24f, 0.24f, 1.0f },
        { 0.24f, 0.24f, 0.24f, 1.0f },
        { 0.24f, 0.24f, 0.24f, 1.0f },
        { 0.24f, 0.24f, 0.24f, 1.0f },
    };
    static const char* sampleOffsetName[numSamples] = 
    {
        "samp0Offset", "samp1Offset", "samp2Offset", "samp3Offset"
    };
    static const char* sampleWeightName[numSamples] = 
    {
        "samp0Weight", "samp1Weight", "samp2Weight", "samp3Weight"
    };

    // prepare the shader variables
    nShader2* blurShader = this->refShader[CONVOLUTIONSHADER].get();
    blurShader->SetTexture(this->varHandle[SRCMAP], srcBuffer);
    
    float pixelWidth  = 1.0f / float(this->offscreenBufferWidth);
    float pixelHeight = 1.0f / float(this->offscreenBufferHeight);
    int curSample;
    for (curSample = 0; curSample < numSamples; curSample++)
    {
        float4 uvOffset = 
        {
            (sampleOffset[curSample][0] + 0.5f) * pixelWidth,
            (sampleOffset[curSample][1] + 0.5f) * pixelHeight,
            0.0f,
            0.0f
        };
        blurShader->SetVector(this->varHandle[SAMPLE0OFFSET + curSample], uvOffset);
        blurShader->SetVector(this->varHandle[SAMPLE0WEIGHT + curSample], sampleWeight[curSample]);
    }

    // render the shader
    gfxServer->SetShader(blurShader);
    gfxServer->Draw();
}

//------------------------------------------------------------------------------
/**
    Render the emissive pass. The emissive pass generates a glowing halo
    effect around light emitting pixels. The halo is generated by a 
    convolution filter which bleeds out the previous contents of the
    emissive buffer over several frames. The positive side effect of
    this should be a "motion blur" around halos.

    @return     returns the offscreen buffer which contains the result
*/
nTexture2*
nMRTSceneServer::RenderEmissivePass()
{
    nGfxServer2* gfxServer = this->refGfxServer.get();

    // is this a odd or even pass?
    nTexture2* srcBuffer;
    nTexture2* dstBuffer;
    if (this->oddFrame)
    {
        srcBuffer = this->refBuffer[EMISSIVEBUFFER0].get();
        dstBuffer = this->refBuffer[EMISSIVEBUFFER1].get();
    }
    else
    {
        srcBuffer = this->refBuffer[EMISSIVEBUFFER1].get();
        dstBuffer = this->refBuffer[EMISSIVEBUFFER0].get();
    }
    this->oddFrame = !this->oddFrame;

    // set the destination buffer as render target and prepare the frame
    gfxServer->SetRenderTarget(dstBuffer);
    gfxServer->BeginScene();
    gfxServer->Clear(nGfxServer2::ALL, 0.0f, 0.0f, 0.0f, 1.0f, 1.0f, 0);

    // set a quad spanning the entire screen as geometry
    gfxServer->SetMesh(0, this->refMesh.get());
    gfxServer->SetVertexRange(0, this->refMesh->GetNumVertices());
    gfxServer->SetIndexRange(0, this->refMesh->GetNumIndices());

    // copy previous frame to destination buffer, and apply a blur filter
    this->ApplyBlurFilter(gfxServer, srcBuffer);

    // render 'new' emissive pixels over the blurred history image,
    // this is the seed for the next frames
    this->RenderShapes('emis');

    // finish the frame and restore the original render target
    gfxServer->EndScene();
    gfxServer->SetRenderTarget(0);

    // return the current destination buffer
    return dstBuffer;
}