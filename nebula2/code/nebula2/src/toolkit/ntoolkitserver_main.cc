//------------------------------------------------------------------------------
//  ntoolkitserver_main.cc
//  (C) 2002 RadonLabs GmbH
//------------------------------------------------------------------------------
#include "toolkit/ntoolkitserver.h"
#include "scene/nmultilayerednode.h"
#include "nature/nswingshapenode.h"

nNebulaScriptClass(nToolkitServer, "nroot");
nToolkitServer* nToolkitServer::Singleton = 0;

//------------------------------------------------------------------------------
/**
*/
nToolkitServer::nToolkitServer():
    isOpen(false)
{
    n_assert(0 == Singleton);
    Singleton = this;
}

//------------------------------------------------------------------------------
/**
*/
nToolkitServer::~nToolkitServer()
{
    n_assert(Singleton);
    Singleton = 0;
}

//------------------------------------------------------------------------------
/**
    Open the scene server. This will parse the render path, initialize
    the shaders assign from the render path, and finally invoke
    nGfxServer2::OpenDisplay().
*/
bool
nToolkitServer::Open()
{
    n_assert(!this->isOpen);

    this->isOpen = true;
    return this->isOpen;
}

//------------------------------------------------------------------------------
/**
    Close the scene server. This will also nGfxServer2::CloseDisplay().
*/
void
nToolkitServer::Close()
{
    n_assert(this->isOpen);
    this->isOpen = false;
}

//------------------------------------------------------------------------------
/**
    This method is called by the "changeshaderparameter"-tcl-command and is used
    by the toolkit remote control to change attributes within the nviewer.

    @param mayaShaderName The Maya name of the shader. It it used to identify
                          the nMaterialNode(s) to which the parameter changes
                          should be applied.
    @param shaderHandler Used to identify the handling function to use, can be
                         "common" or "particle2".
    @param paramID An unique identifier for the parameter that should be
                   changed, does not need to be globally unique, but unique
                   for one shader.
    @param paramValue A string containing the new value for the parameter.
*/
nString
nToolkitServer::ChangeShaderParameter(const nString& mayaShaderName,
                                      const nString& shaderHandler,
                                      const nString& paramID,
                                      const nString& paramValue)
{
    // iterate through nodelist and apply changes
    bool result = true;
    nNodeList* nodeList = nNodeList::Instance();
    uint nodeCount = nodeList->GetCount();
    uint i;
    for (i = 0; i < nodeCount; i++)
    {
        nTransformNode* node = nodeList->GetNodeAt(i);
        result &= this->ChangeShaderParameterOnNode(node, mayaShaderName,
                                                    shaderHandler, paramID,
                                                    paramValue);
    }
    if (!result)
    {
        return "false";
    }
    return "true";
}

//------------------------------------------------------------------------------
/**
    Recursive iteration through the node hierarchy.
*/
bool
nToolkitServer::ChangeShaderParameterOnNode(nRoot* node,
                                            const nString& mayaShaderName,
                                            const nString& shaderHandler,
                                            const nString& paramID,
                                            const nString& paramValue)
{
    bool result = true;
    if (node->IsA("nmaterialnode"))
    {
        nMaterialNode* matNode = (nMaterialNode*) node;
        if (matNode->GetMayaShaderName() == mayaShaderName)
        {
            if ("common" == shaderHandler)
            {
                // common shader attributes
                this->ChangeTypeCommon(matNode, paramID, paramValue);
            }
            else if ("particle2" == shaderHandler)
            {
                // particle2 shader attributes
                this->ChangeTypeParticle2(matNode, paramID, paramValue);
            }
            else if ("swing" == shaderHandler)
            {
                // particle2 shader attributes
                this->ChangeTypeSwing(matNode, paramID, paramValue);
            }
            // default shader attributes
            this->ChangeTypeDefault(matNode, paramID, paramValue);
        }
    }
    // apply changes to sub-nodes
    nRoot* curChild;
    for (curChild = (nRoot*) node->GetHead();
         curChild;
         curChild = (nRoot*) curChild->GetSucc())
    {
        result &= this->ChangeShaderParameterOnNode(curChild, mayaShaderName,
                                                    shaderHandler, paramID,
                                                    paramValue);
    }
    return result;
}

//------------------------------------------------------------------------------
/**
*/
bool
nToolkitServer::ChangeTypeCommon(nMaterialNode* node,
                                 const nString& paramID,
                                 const nString& paramValue)
{
    bool result = true;

    nString fileName;

    n_printf("Received remote command : %s %s\n",
             paramID.Get(), paramValue.Get());

    if ("MatDiffuse" == paramID)
    {
        // Material Diffuse
        if (node->HasParam(nShaderState::MatDiffuse))
        {
            node->SetVector(nShaderState::MatDiffuse, paramValue.AsVector4());
        }
    }
    else if ("MatEmissive" == paramID)
    {
        // Emissive Color
        if (node->HasParam(nShaderState::MatEmissive))
        {
            node->SetVector(nShaderState::MatEmissive, paramValue.AsVector4());
        }
    }
    else if ("MatSpecular" == paramID)
    {
        // Specular Color
        if (node->HasParam(nShaderState::MatSpecular))
        {
            node->SetVector(nShaderState::MatSpecular, paramValue.AsVector4());
        }
    }
    else if ("MatEmissiveIntensity" == paramID)
    {
        // Emissive Intensity
        if (node->HasParam(nShaderState::MatEmissiveIntensity))
        {
            node->SetFloat(nShaderState::MatEmissiveIntensity, paramValue.AsFloat());
        }
    }
    else if ("MatSpecularPower" == paramID)
    {
        // Specular Intensity
        if (node->HasParam(nShaderState::MatSpecularPower))
        {
            node->SetFloat(nShaderState::MatSpecularPower, paramValue.AsFloat());
        }
    }
    else if ("BumpScale" == paramID)
    {
        if (node->HasParam(nShaderState::BumpScale))
        {
            node->SetFloat(nShaderState::BumpScale, paramValue.AsFloat());
        }
    }
    else if ("AlphaSrcBlend" == paramID)
    {
        if (node->HasParam(nShaderState::AlphaSrcBlend))
        {
            node->SetInt(nShaderState::AlphaSrcBlend, paramValue.AsInt());
        }
    }
    else if ("AlphaDstBlend" == paramID)
    {
        if (node->HasParam(nShaderState::AlphaDstBlend))
        {
            node->SetInt(nShaderState::AlphaDstBlend, paramValue.AsInt());
        }
    }
    else if ("DiffMap0" == paramID)
    {
        fileName = nFileServer2::Instance()->ManglePath(paramValue);
        if (nFileServer2::Instance()->FileExists(fileName))
        {
            if (node->HasParam(nShaderState::DiffMap0))
            {
                node->SetTexture(nShaderState::DiffMap0, fileName.Get());
            }
        }
    }
    else if ("BumpMap0" == paramID)
    {
        fileName = nFileServer2::Instance()->ManglePath(paramValue);
        if (nFileServer2::Instance()->FileExists(fileName))
        {
            if (node->HasParam(nShaderState::BumpMap0))
            {
                node->SetTexture(nShaderState::BumpMap0, fileName.Get());
            }
        }
    }
    else if (paramID.FindStringIndex("UVStretch", 0) != -1)
    {
        // UV Stretch of MultiLayeredShader
        // FIXME: currently only working with dx9
        if (node->IsA("nmultilayerednode"))
        {
            int maxTexturesPerShader = 5;
            nMultiLayeredNode* mlNode = (nMultiLayeredNode*) node;

            // extract map index from parameterID
            int mapIndex = (paramID.ExtractRange(11, paramID.Length() - 11)).AsInt();
            float uv = paramValue.AsFloat();

            n_printf("setting to %4.4f\n", uv);

            // now we need to recalculate the correct index
            int dx9pass = mapIndex / maxTexturesPerShader;
            int dx9offset = mapIndex % maxTexturesPerShader;
            if (mlNode->GetPartIndex() == dx9pass)
            {
                mlNode->SetUVStretch(dx9offset, uv);
            }
        }
    }
    else
    {
        nShaderState::Param param = nShaderState::StringToParam(paramID.Get());
        if (param != nShaderState::InvalidParameter && node->HasParam(param))
        {
            switch(param)
            {
            case nShaderState::SkyBottom:
            case nShaderState::SunFlat:
            case nShaderState::SunRange:
            case nShaderState::Lightness:
            case nShaderState::Density:
            case nShaderState::Glow:
            case nShaderState::Map0uvRes:
            case nShaderState::Map1uvRes:
            case nShaderState::BumpFactor:
            case nShaderState::Weight:
                node->SetFloat(param, paramValue.AsFloat());
                break;
            case nShaderState::SunColor:
            case nShaderState::CloudMod:
            case nShaderState::CloudPos:
            case nShaderState::CloudGrad:
            case nShaderState::Brightness:
            case nShaderState::Saturation:
            case nShaderState::TopColor:
            case nShaderState::BottomColor:
            case nShaderState::Move:
            case nShaderState::Position:
            case nShaderState::ScaleVector:
                node->SetVector(param, paramValue.AsVector4());
                break;
            }
        }
    }

    return result;
}

//------------------------------------------------------------------------------
/**
    -01-Dec-06  kims  which are for "RenderPri" and "LockViewer" shader parameter.
                      Thans kaikai for the patch.
*/
bool
nToolkitServer::ChangeTypeDefault(nMaterialNode* node,
                                 const nString& paramID,
                                 const nString& paramValue)
{
    bool result = true;

    n_printf("Received remote command : %s %s\n",
        paramID.Get(), paramValue.Get());

    if ("RenderPri" == paramID)
    {
        node->SetRenderPri(paramValue.AsInt());
    }
    else if ("LockViewer" == paramID)
    {
        node->SetLockViewer(paramValue.AsBool());
    }

    return result;
}

//------------------------------------------------------------------------------
/**
    -01-Dec-06  kims  which are for nswingnode. Thans kaikai for the patch.
*/
bool
nToolkitServer::ChangeTypeSwing(nMaterialNode* node,
                                  const nString& paramID,
                                  const nString& paramValue)
{
    bool result = true;

    if (node->IsA("nswingshapenode"))
    {
        nSwingShapeNode *swingShapeNode = (nSwingShapeNode *)node;
        n_printf("Received remote command : %s %s\n",
            paramID.Get(), paramValue.Get());

        if ("SwingAngle" == paramID)
        {
            swingShapeNode->SetSwingAngle(paramValue.AsFloat());
        }
        else if ("SwingTime" == paramID)
        {
            swingShapeNode->SetSwingTime(paramValue.AsFloat());
        }
    }

    return result;
}

//------------------------------------------------------------------------------
/**
*/
nEnvelopeCurve
nToolkitServer::AsEnvelopeCurve(const nString& value)
{
    nArray<nString> tokens;
    value.Tokenize(" ", tokens);
    n_assert(tokens.Size() == 9);
    return nEnvelopeCurve(tokens[0].AsFloat(), tokens[1].AsFloat(),
                          tokens[2].AsFloat(), tokens[3].AsFloat(),
                          tokens[4].AsFloat(), tokens[5].AsFloat(),
                          tokens[6].AsFloat(), tokens[7].AsFloat(),
                          tokens[8].AsInt());
}

//------------------------------------------------------------------------------
/**
*/
nVector3EnvelopeCurve
nToolkitServer::AsColorCurve(const nString& value)
{
    nArray<nString> tokens;
    value.Tokenize(" ", tokens);
    n_assert(tokens.Size() == 14);
    return nVector3EnvelopeCurve(vector3(tokens[0].AsFloat(), tokens[1].AsFloat(), tokens[2].AsFloat()),
                                 vector3(tokens[3].AsFloat(), tokens[4].AsFloat(), tokens[5].AsFloat()),
                                 vector3(tokens[6].AsFloat(), tokens[7].AsFloat(), tokens[8].AsFloat()),
                                 vector3(tokens[9].AsFloat(), tokens[10].AsFloat(), tokens[11].AsFloat()),
                                 tokens[12].AsFloat(), tokens[13].AsFloat());
}

//------------------------------------------------------------------------------
/**
    -04-Dec-06  kims  Changed that particles can be emitted on a surface.
    -29-Nov-06  kims  Changed to be compatible the particle2 shader.
                      Thank kaikai for the patch.
*/
bool
nToolkitServer::ChangeTypeParticle2(nMaterialNode* node, const nString& paramID,
                                    const nString& paramValue)
{
    bool result = true;

    if (node->IsA("nparticleshapenode2"))
    {
        nParticleShapeNode2* partNode = (nParticleShapeNode2*) node;

        if ("Gravity" == paramID)
        {
            partNode->SetGravity(paramValue.AsFloat());
        }
        else if ("ActivityDistance" == paramID)
        {
            partNode->SetActivityDistance(paramValue.AsFloat());
        }
        else if ("ParticleStretch" == paramID)
        {
            partNode->SetParticleStretch(paramValue.AsFloat());
        }
        else if ("StretchDetail" == paramID)
        {
            partNode->SetStretchDetail(paramValue.AsInt());
        }
        else if ("ParticleVelocityRandomize" == paramID)
        {
            partNode->SetParticleVelocityRandomize(paramValue.AsFloat());
        }
        else if ("StartRotationMin" == paramID)
        {
            partNode->SetStartRotationMin(paramValue.AsFloat());
        }
        else if ("StartRotationMax" == paramID)
        {
            partNode->SetStartRotationMax(paramValue.AsFloat());
        }
        else if ("ParticleRotationRandomize" == paramID)
        {
            partNode->SetParticleRotationRandomize(paramValue.AsFloat());
        }
        else if ("ParticleSizeRandomize" == paramID)
        {
            partNode->SetParticleSizeRandomize(paramValue.AsFloat());
        }
        else if ("EmissionFrequency" == paramID)
        {
            partNode->SetCurve(nParticle2Emitter::EmissionFrequency,
                               this->AsEnvelopeCurve(paramValue));
        }
        else if ("ParticleLifeTime" == paramID)
        {
            partNode->SetCurve(nParticle2Emitter::ParticleLifeTime,
                               this->AsEnvelopeCurve(paramValue));
        }
        else if ("ParticleStartVelocity" == paramID)
        {
            partNode->SetCurve(nParticle2Emitter::ParticleStartVelocity,
                               this->AsEnvelopeCurve(paramValue));
        }
        else if ("ParticleSpreadMax" == paramID)
        {
            partNode->SetCurve(nParticle2Emitter::ParticleSpreadMax,
                               this->AsEnvelopeCurve(paramValue));
        }
        else if ("ParticleSpreadMin" == paramID)
        {
            partNode->SetCurve(nParticle2Emitter::ParticleSpreadMin,
                               this->AsEnvelopeCurve(paramValue));
        }
        else if ("ParticleRotationVelocity" == paramID)
        {
            partNode->SetCurve(nParticle2Emitter::ParticleRotationVelocity,
                               this->AsEnvelopeCurve(paramValue));
        }
        else if ("ParticleScale" == paramID)
        {
            partNode->SetCurve(nParticle2Emitter::ParticleScale,
                               this->AsEnvelopeCurve(paramValue));
        }
        else if ("ParticleAlpha" == paramID)
        {
            partNode->SetCurve(nParticle2Emitter::ParticleAlpha,
                               this->AsEnvelopeCurve(paramValue));
        }
        else if ("ParticleAirResistance" == paramID)
        {
            partNode->SetCurve(nParticle2Emitter::ParticleAirResistance,
                               this->AsEnvelopeCurve(paramValue));
        }
        else if ("ParticleVelocityFactor" == paramID)
        {
            partNode->SetCurve(nParticle2Emitter::ParticleVelocityFactor,
                               this->AsEnvelopeCurve(paramValue));
        }
        else if ("ParticleMass" == paramID)
        {
            partNode->SetCurve(nParticle2Emitter::ParticleMass,
                               this->AsEnvelopeCurve(paramValue));
        }
        else if ("TimeManipulator" == paramID)
        {
            partNode->SetCurve(nParticle2Emitter::TimeManipulator,
                               this->AsEnvelopeCurve(paramValue));
        }
        else if ("TileTexture" == paramID)
        {
            partNode->SetTileTexture(paramValue.AsInt());
        }
        else if ("StretchToStart" == paramID)
        {
            partNode->SetStretchToStart(paramValue.AsBool());
        }
        else if ("ViewAngleFade" == paramID)
        {
            partNode->SetViewAngleFade(paramValue.AsBool());
        }
        else if ("RenderOldestFirst" == paramID)
        {
            partNode->SetRenderOldestFirst(paramValue.AsBool());
        }
        else if ("RandomRotDir" == paramID)
        {
            partNode->SetRandomRotDir(paramValue.AsBool());
        }
        else if ("ParticleRGB" == paramID)
        {
            partNode->SetRGBCurve(this->AsColorCurve(paramValue));
        }
        else if ("Loop" == paramID)
        {
            partNode->SetLoop(paramValue.AsBool());
        }
        else if ("EmissionDuration" == paramID)
        {
            partNode->SetEmissionDuration(paramValue.AsFloat());
        }
        else if ("PrecalcTime" == paramID)
        {
            partNode->SetPrecalcTime(paramValue.AsFloat());
        }
        else if ("StartDelay" == paramID)
        {
            partNode->SetStartDelay(paramValue.AsFloat());
        }
        else if ("EmitOnSurface" == paramID)
        {
            partNode->SetEmitOnSurface(paramValue.AsBool());
        }
    }
    else
    {
        result = false;
    }
    return result;
}

//------------------------------------------------------------------------------
/**
    Clear existing scene objects and load given objects.

    -03-Nov-06  kims Created.

    @param objPath - .n2 objects to load.
    
*/
void
nToolkitServer::DoHotLoading(const nString& objPath)
{
    nNodeList* nodeList = nNodeList::Instance();

    nFileServer2 *fileServer = nKernelServer::Instance()->GetFileServer();
    n_assert(fileServer);
    if (!fileServer->IsA("nramfileserver"))
    {
        nKernelServer::Instance()->ReplaceFileServer("nramfileserver");
        n_assert(nKernelServer::Instance()->GetFileServer());
    }
    // remove all objects under '/usr/scene'.
    nodeList->Clear();

    // add default entry.(default scene light is created)
    nodeList->AddDefaultEntry();

    // load object with the given path.
    nodeList->LoadObject(objPath);
}
