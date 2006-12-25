//------------------------------------------------------------------------------
//  nglslshader_main.cc
//  23-Mar-2005 Haron
//------------------------------------------------------------------------------

#include "opengl/nglslshader.h"
#include "opengl/nglshaderinclude.h"

#include "opengl/nglserver2.h"
#include "opengl/ngltexture.h"
#include "opengl/nglextensionserver.h"

#include "opengl/npassstate.h"

#include "gfx2/nshaderparams.h"
#include "kernel/nfileserver2.h"
#include "kernel/nfile.h"

#include "tinyxml/tinyxml.h"

nNebulaClass(nGLSLShader, "nshader2");

void
n_glsltrace(GLhandleARB obj, const nString& msg)
{
    bool errorPresent = false;
    uint errNum = 0;
    GLenum error = glGetError();
    nString message;

    while (error != GL_NO_ERROR)
    {
        errorPresent = true;

        if (errNum < 20)
        {
            switch (error)
            {
                case GL_OUT_OF_MEMORY:
                    message += "<GL_OUT_OF_MEM>";
                    break;
                case GL_INVALID_ENUM:
                    message += "<GL_INVALID_ENUM>";
                    break;
                case GL_INVALID_VALUE:
                    message += "<GL_INVALID_VALUE>";
                    break;
                case GL_INVALID_OPERATION:
                    message += "<GL_INVALID_OPERATION>";
                    break;
                default:
                    message += "<GL_ERROR_TYPE: ";
                    message.AppendInt(error);
                    message += ">";
            }
        }
        else
        {
            message += "...";
            break;
        }

        errNum++;
        error = glGetError();
    }

    if (errorPresent)
    {
        if (obj > 0)
        {
            int logLength = 0;

            glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLength);

            if (logLength > 0)
            {
                GLcharARB* infoLog = new GLcharARB[logLength];

                glGetInfoLogARB(obj, logLength, NULL, infoLog);

                n_message("\n%s\n    Error: %s\n    GLSL Error: %s\n", msg.Get(), message.Get(), infoLog);

                delete [] infoLog;
            }
        }
        else
        {
            n_message("\n%s\n    Error: %s\n", msg.Get(), message.Get());
        }
    }
}

void
n_assert_glslstatus(GLhandleARB obj, const nString& msg, GLenum param)
{
    GLint res;
    glGetObjectParameterivARB(obj, param, &res);

    if (res == GL_FALSE)
    {
        GLint logLen = 0;
        GLint charsWritten = 0;

        glGetObjectParameterivARB(obj, GL_OBJECT_INFO_LOG_LENGTH_ARB, &logLen);

        char *log = n_new_array(char, logLen);

        glGetInfoLogARB(obj, logLen, &charsWritten, log);
        n_error("%s\n[%s]", msg.Get(), log);

        n_delete_array(log);
    }
}

//------------------------------------------------------------------------------
/**
*/
nGLSLShader::nGLSLShader() :
    hasBeenValidated(false),
    didNotValidate(false),
    //uniformsUpdated(false),
    activeTechniqueIdx(-1),
    programObj(0),
    vertShader(0),
    fragShader(0)
{
    memset(this->parameterHandles, -1, sizeof(this->parameterHandles));
}

//------------------------------------------------------------------------------
/**
*/
nGLSLShader::~nGLSLShader()
{
    //    if (!deviceInit) return;
    if (this->IsLoaded())
    {
        this->Unload();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::UnloadResource()
{
    n_assert(this->IsLoaded());

    // if this is the currently set shader, unlink from gfx server
    if (nGfxServer2::Instance()->GetShader() == this)
    {
        nGfxServer2::Instance()->SetShader(0);
    }

    glDetachObjectARB(this->programObj, this->vertShader);
    glDeleteObjectARB(this->vertShader);

    glDetachObjectARB(this->programObj, this->fragShader);
    glDeleteObjectARB(this->fragShader);

    glDeleteObjectARB(this->programObj);

    //uniformsUpdated = false;

    // clear current parameter settings
    this->curParams.Clear();

    this->SetState(Unloaded);
}

//------------------------------------------------------------------------------
/**
Load GLSL-shader files.
*/
bool
nGLSLShader::LoadResource()
{
    n_assert(!this->IsLoaded());
    //n_assert(deviceInit);

    //nGLShaderInclude si;
    //nString fname = this->GetFilename().ExtractDirName();
    nString src;

    //fname.StripExtension();

    n_printf("\nStart shader <%s> loading...\n", this->GetFilename().Get());

    // parse XML
    nString path = nFileServer2::Instance()->ManglePath(this->GetFilename());
    nString shdDir = path.ExtractDirName();
    TiXmlDocument* xmlDocument = n_new(TiXmlDocument);

    if (xmlDocument->LoadFile(path.Get()))
    {
        TiXmlElement* child;
        TiXmlHandle docHandle(xmlDocument);
        TiXmlElement* elmShader = docHandle.FirstChildElement("shader").Element();
        n_assert(elmShader);

        this->programObj = glCreateProgramObjectARB(); 

        for (child = elmShader->FirstChildElement(); child; child = child->NextSiblingElement())
        {
            if (child->Value() == nString("source"))
            {
                if (child->Attribute("type") == nString("vertex"))
                {
                    this->vertShader = this->CreateGLSLShader(VERTEX, shdDir + child->Attribute("path"));
                    glAttachObjectARB(this->programObj, this->vertShader);
                }
                else if (child->Attribute("type") == nString("fragment"))
                {
                    this->fragShader = this->CreateGLSLShader(FRAGMENT, shdDir + child->Attribute("path"));
                    glAttachObjectARB(this->programObj, this->fragShader);
                }
                else
                {
                    n_message("nGLSLShader::LoadResource(): Unsupported shader source type <%s>.", child->Attribute("type"));
                }
            }
            else if (child->Value() == nString("technique"))
            {
                TiXmlElement* pass;

                for (pass = child->FirstChildElement(); pass; pass = pass->NextSiblingElement())
                {
                    if (pass->Value() == nString("pass"))
                    {
                        TiXmlElement* param;
                        int paramCount = 0;
                        GLuint list = glGenLists(1);

                        glNewList(list, GL_COMPILE);
                        for (param = pass->FirstChildElement(); param; param = param->NextSiblingElement())
                        {
                            if (param->Value() == nString("param"))
                            {
                                if (this->ParsePassParam(param->Attribute("name"), nString(param->Attribute("value"))))
                                {
                                    paramCount++;
                                }
                            }
                            else
                            {
                                n_message("nGLSLShader::LoadResource(): Unsupported pass tag <%s>. Only <param> tag supported.", param->Value());
                            }
                        }
                        glEndList();

                        if (paramCount > 0)
                        {
                        }
                        else
                        {
                            glDeleteLists(list, 1);
                        }
                    }
                    else
                    {
                        n_message("nGLSLShader::LoadResource(): Unsupported technique tag <%s>. Only <pass> tag supported.", pass->Value());
                    }
                }
            }
            else
            {
                n_message("nGLSLShader::LoadResource(): Unsupported shader tag <%s>.", child->Value());
            }
        }

        glLinkProgramARB(this->programObj);
        n_assert_glslstatus(this->programObj, "nGLSLShader::LoadResource(): Program linking Error.", GL_OBJECT_LINK_STATUS_ARB);

        // success
        this->hasBeenValidated = false;
        this->didNotValidate = false;
        this->SetState(Valid);

        // validate the effect
        this->ValidateEffect();

        //uniformsUpdated = false;
        this->UpdateParameterHandles();

        src.Set("nGLSLShader::LoadResource(");
        src.Append(this->GetFilename());
        src.Append(").");

        n_gltrace(src.Get());
        n_glsltrace(this->programObj, src.Get());

        return true;
    }

    n_delete(xmlDocument);
    n_message("nGLSLShader::LoadResource(): Can't load shader file.");

    return false;
}

//------------------------------------------------------------------------------
/**
*/
uint
nGLSLShader::CreateGLSLShader(GLSLShaderType type, const nString& path)
{
    nGLShaderInclude si;
    GLuint shd;

    if (si.Begin(path))
    {
        const char *shaderStrings[1];
        nString src = si.GetSource();

        si.End();

        shaderStrings[0] = src.Get();

        switch (type)
        {
        case VERTEX:   shd = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);   break;
        case FRAGMENT: shd = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB); break;
        }
        n_assert(shd != 0);

        glShaderSourceARB(shd, 1, shaderStrings, NULL);
        glCompileShaderARB(shd);

        nString msg("nGLSLShader::CreateGLSLShader(");
        msg += path + "): Shader Compile Error.";
        n_assert_glslstatus(shd, msg, GL_OBJECT_COMPILE_STATUS_ARB);

        //glAttachObjectARB(this->programObj, shd);
    }

    return shd;
}

//------------------------------------------------------------------------------
/**
*/
bool
nGLSLShader::ParsePassParam(const char* name, nString& val)
{
    val.ToLower();
    switch (nPassState::StringToParam(name))
    {
    case nPassState::ZWriteEnable:
        return true;

    case nPassState::ZEnable:
        if (val == nString("true"))
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
        return true;         

    case nPassState::ZFunc:
        if (val == nString("lessequal"))
        {
            glDepthFunc(GL_LEQUAL);
        }
        // TODO: add other functions
        return true;         

    case nPassState::ColorWriteEnable:
        return true;

    case nPassState::AlphaBlendEnable:
        return true;

    case nPassState::SrcBlend:
        return true;        

    case nPassState::DestBlend:
        return true;       

    case nPassState::AlphaTestEnable:
        return true; 

    case nPassState::StencilEnable:
        return true;   

    case nPassState::CullMode:
        // TODO: test this
        if (val == nString("cw"))
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_FRONT);
            //glFrontFace(GL_CW);
        }
        else if (val == nString("ccw"))
        {
            glEnable(GL_CULL_FACE);
            glCullFace(GL_BACK);
            //glFrontFace(GL_CCW);
        }
        else
        {
            glDisable(GL_CULL_FACE);
        }
        return true; 

    default:
        n_message("nGLSLShader::ParsePassParam(): Unsupported pass parameter <%s>.", name);
        return false;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetBool(nShaderState::Param p, bool val)
{
    n_assert(p < nShaderState::NumParameters);

    //n_printf("SetBool: %s\n", nShaderState::ParamToString(p));
    this->curParams.SetArg(p, nShaderArg(val));
    glUniform1iARB(this->parameterHandles[p], val ? 1 : 0);

    n_gltrace("nGLSLShader::SetBool().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetBoolArray(nShaderState::Param p, const bool* arr, int count)
{
    n_assert(p < nShaderState::NumParameters);

    GLint* ba = n_new_array(GLint, count);
    int i;

    for (i = 0; i < count; i++)
    {
        ba[i] = arr[i] ? 1 : 0;
    }
    
    glUniform1ivARB(this->parameterHandles[p], count, ba);
    n_delete_array(ba);

    n_gltrace("nGLSLShader::SetBoolArray().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetInt(nShaderState::Param p, int val)
{
    n_assert(p < nShaderState::NumParameters);

    //n_printf("SetInt: %s\n", nShaderState::ParamToString(p));
    this->curParams.SetArg(p, nShaderArg(val));
    glUniform1iARB(this->parameterHandles[p], val);

    n_gltrace("nGLSLShader::SetInt().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetIntArray(nShaderState::Param p, const int* arr, int count)
{
    n_assert(p < nShaderState::NumParameters);

    glUniform1ivARB(this->parameterHandles[p], count, arr);

    n_gltrace("nGLSLShader::SetIntArray().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetFloat(nShaderState::Param p, float val)
{
    n_assert(p < nShaderState::NumParameters);

    //n_printf("SetFloat: %s\n", nShaderState::ParamToString(p));
    this->curParams.SetArg(p, nShaderArg(val));
    glUniform1fARB(this->parameterHandles[p], val);

    n_gltrace("nGLSLShader::SetFloat().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetFloatArray(nShaderState::Param p, const float* arr, int count)
{
    n_assert(p < nShaderState::NumParameters);

    glUniform1fvARB(this->parameterHandles[p], count, arr);

    n_gltrace("nGLSLShader::SetFloatArray().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetVector4(nShaderState::Param p, const vector4& val)
{
    n_assert(p < nShaderState::NumParameters);

    //n_printf("SetVector4: %s\n", nShaderState::ParamToString(p));
    this->curParams.SetArg(p, nShaderArg(*(nFloat4*)&val));
    glUniform4fvARB(this->parameterHandles[p], 1, (float*) &val);

    n_gltrace("nGLSLShader::SetVector4().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetVector3(nShaderState::Param p, const vector3& val)
{
    n_assert(p < nShaderState::NumParameters);

    //n_printf("SetVector3: %s\n", nShaderState::ParamToString(p));
    this->curParams.SetArg(p, nShaderArg(*(nFloat4*)&val));
    glUniform3fvARB(this->parameterHandles[p], 1, (float*) &val);

    n_gltrace("nGLSLShader::SetVector3().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetFloat4(nShaderState::Param p, const nFloat4& val)
{
    n_assert(p < nShaderState::NumParameters);

    //n_printf("SetFloat4: %s\n", nShaderState::ParamToString(p));
    this->curParams.SetArg(p, nShaderArg(val));
    glUniform4fvARB(this->parameterHandles[p], 1, (float*) &val);

    n_gltrace("nGLSLShader::SetFloat4().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetFloat4Array(nShaderState::Param p, const nFloat4* arr, int count)
{
    n_assert(p < nShaderState::NumParameters);

    // TODO: check "(float*) arr" expression correctness
    glUniform4fvARB(this->parameterHandles[p], count, (float*) arr);

    n_gltrace("nGLSLShader::SetFloat4Array().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetVector4Array(nShaderState::Param p, const vector4* arr, int count)
{
    n_assert(p < nShaderState::NumParameters);

    // TODO: check "(float*) arr" expression correctness
    glUniform4fvARB(this->parameterHandles[p], count, (float*) arr);

    n_gltrace("nGLSLShader::SetVector4Array().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetMatrix(nShaderState::Param p, const matrix44& val)
{
    n_assert(p < nShaderState::NumParameters);

    //GLint pp = glGetUniformLocationARB(this->programObj, nShaderState::ParamToString(p));

    //n_printf("SetMatrix: %s\n", nShaderState::ParamToString(p));
    this->curParams.SetArg(p, nShaderArg(&val));
    glUniformMatrix4fvARB(this->parameterHandles[p], 1, GL_FALSE, (const GLfloat*)val.m);

    nString msg;
    msg.Format("nGLSLShader(%s)::SetMatrix(%s:%d).", this->GetFilename().Get(), nShaderState::ParamToString(p), this->parameterHandles[p]);
    //n_gltrace(msg.Get());
    n_glsltrace(this->programObj, msg.Get());

    //printf("%s\n", msg.Get());

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetMatrixArray(nShaderState::Param p, const matrix44* arr, int count)
{
    n_assert(p < nShaderState::NumParameters);

    // TODO: check "(float*) arr" expression correctness
    glUniformMatrix4fvARB(this->parameterHandles[p], count, GL_FALSE, (float*) arr);

    n_gltrace("nGLSLShader::SetMatrixArray().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetMatrixPointerArray(nShaderState::Param p, const matrix44** arr, int count)
{
    n_assert(p < nShaderState::NumParameters);

    // TODO: check "(float*) *arr" expression correctness
    glUniformMatrix4fvARB(this->parameterHandles[p], count, GL_FALSE, (float*) *arr);

    n_gltrace("nGLSLShader::SetMatrixPointerArray().");

#ifdef __NEBULA_STATS__
    //this->refGfxServer->statsNumRenderStateChanges++;
#endif
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::SetTexture(nShaderState::Param p, nTexture2* tex)
{
    n_assert(tex);
    n_assert(p < nShaderState::NumParameters);

    //n_printf("SetTexture: %s\n", nShaderState::ParamToString(p));

    if ((!this->curParams.IsParameterValid(p)) ||
        (this->curParams.IsParameterValid(p) && (this->curParams.GetArg(p).GetTexture() != tex)))
    {
        this->curParams.SetArg(p, nShaderArg(tex));
        glUniform1iARB(this->parameterHandles[p], (GLint)((nGLTexture*)tex)->GetTexID());

        n_gltrace("nGLSLShader::SetTexture().");

#ifdef __NEBULA_STATS__
        //this->refGfxServer->statsNumTextureChanges++;
#endif
    }
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::BeginParamUpdate()
{
    glUseProgramObjectARB(this->programObj);
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::EndParamUpdate()
{
    glUseProgramObjectARB(0);
}

//------------------------------------------------------------------------------
/**
Set a whole shader parameter block at once. This is slightly faster
(and more convenient) then setting single parameters.
*/
void
nGLSLShader::SetParams(const nShaderParams& params)
{
#ifdef __NEBULA_STATS__
    //nGLServer2* gfxServer = this->refGfxServer.get();
#endif
    int numValidParams = params.GetNumValidParams();
    int i;

    glUseProgramObjectARB(this->programObj);

    for (i = 0; i < numValidParams; i++)
    {
        nShaderState::Param curParam = params.GetParamByIndex(i);

        // parameter used in shader?
        GLint handle = this->parameterHandles[curParam];
        if (handle != -1)
        {
            // avoid redundant state switches
            const nShaderArg& curArg = params.GetArgByIndex(i);
            if ((!this->curParams.IsParameterValid(curParam)) ||
                (!(curArg == this->curParams.GetArg(curParam))))
            {
                this->curParams.SetArg(curParam, curArg);
                switch (curArg.GetType())
                {
                case nShaderState::Void:
#ifdef __NEBULA_STATS__
                    //gfxServer->statsNumRenderStateChanges++;
#endif
                    break;

                case nShaderState::Int:
                    glUniform1iARB(handle, curArg.GetInt());
#ifdef __NEBULA_STATS__
                    //gfxServer->statsNumRenderStateChanges++;
#endif
                    break;

                case nShaderState::Float:
                    glUniform1fARB(handle, curArg.GetFloat());
#ifdef __NEBULA_STATS__
                    //gfxServer->statsNumRenderStateChanges++;
#endif
                    break;

                case nShaderState::Float4:
                    glUniform4fvARB(handle, 1, (float*) &(curArg.GetFloat4()));
#ifdef __NEBULA_STATS__
                    //gfxServer->statsNumRenderStateChanges++;
#endif
                    break;

                case nShaderState::Matrix44:
                    glUniformMatrix4fvARB(handle, 1, GL_FALSE, (float*) curArg.GetMatrix44());
#ifdef __NEBULA_STATS__
                    //gfxServer->statsNumRenderStateChanges++;
#endif
                    break;

                case nShaderState::Texture:
                    glUniform1iARB(handle, (GLint)((nGLTexture*)curArg.GetTexture())->GetTexID());
#ifdef __NEBULA_STATS__
                    //gfxServer->statsNumTextureChanges++;
#endif
                    break;
                }
                //n_error("nGLSLShader::SetParams(): Error while setting parameter.");
            }
        }
    }

    glUseProgramObjectARB(0);

    //n_gltrace("nGLSLShader::SetParams().");
    n_glsltrace(this->programObj, "nGLSLShader::SetParams().");
}

//------------------------------------------------------------------------------
/**
Update the parameter handles table which maps nShader2 parameters to
GLSL parameter handles.
*/
void
nGLSLShader::UpdateParameterHandles()
{
    //if (uniformsUpdated) return;

    memset(this->parameterHandles, -1, sizeof(this->parameterHandles));

    n_printf("Start parameters lookup\n");

    // TODO: trace all parameters in shaders, not only nebula ones
    int i, n;
    GLcharARB* uniformName;
    GLsizei maxLen, len;
    GLint sz;
    GLenum tp;

    glUseProgramObjectARB(this->programObj);

    glGetObjectParameterivARB(this->programObj, GL_OBJECT_ACTIVE_UNIFORMS_ARB, &n);
    glGetObjectParameterivARB(this->programObj, GL_OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB, &maxLen);

    if (n > 0)
    {
        n_printf("  Shader contain:");
        uniformName = n_new_array(GLcharARB, maxLen);
        for (i = 0; i < n; i++)
        {
            glGetActiveUniformARB(this->programObj, i, maxLen, &len, &sz, &tp, uniformName);
            n_printf(" %s", uniformName);
        }
        n_delete_array(uniformName);
        n_printf("\n");
        
        GLint p;
        const char* pname;
        for (i = 0; i < nShaderState::NumParameters; i++)
        {
            pname = nShaderState::ParamToString((nShaderState::Param)i);
            p = glGetUniformLocationARB(this->programObj, pname);

            if (p != -1)
            {
                n_printf("  %4d: %s\n", p, pname);
            }

            //n_printf("  Found: %s = %d\n", pname, p);
            this->parameterHandles[i] = p;
        }
    }
    else
    {
        n_printf("  There are no parameters in this shader.\n");
    }

    glUseProgramObjectARB(0);

    n_printf("End parameters lookup\n");

    //uniformsUpdated = true;

    //n_gltrace("nGLSLShader::UpdateParameterHandles().");
    n_glsltrace(this->programObj, "nGLSLShader::UpdateParameterHandles().");
}

//------------------------------------------------------------------------------
/**
Return true if parameter is used by effect.
*/
bool
nGLSLShader::IsParameterUsed(nShaderState::Param p)
{
    n_assert(p < nShaderState::NumParameters);
    return (0 <= this->parameterHandles[p]);
}

//------------------------------------------------------------------------------
/**
Find the first valid technique and set it as current.
This sets the hasBeenValidated and didNotValidate members
*/
void
nGLSLShader::ValidateEffect()
{
    n_assert(!this->hasBeenValidated);
    //n_assert(deviceInit);

    //int res;

    this->hasBeenValidated = true;
    this->didNotValidate = true;

    glValidateProgramARB(this->programObj);
    n_assert_glslstatus(this->programObj, "nGLSLShader() warning: shader did not validated!", GL_OBJECT_VALIDATE_STATUS_ARB);

    //this->hasBeenValidated = true;
    this->didNotValidate = false;
    //this->UpdateParameterHandles();

    //n_gltrace("nGLSLShader::ValidateEffect().");
    n_glsltrace(this->programObj, "nGLSLShader::ValidateEffect().");
}

//------------------------------------------------------------------------------
/**
begin shader rendering
*/
int
nGLSLShader::Begin(bool saveState)
{
    // check if we already have been validated, if not, find the first
    // valid technique and set it as current
    if (!this->hasBeenValidated)
    {
        this->ValidateEffect();
    }

    if (this->didNotValidate)
    {
        //TODO: FIXME - validation not working!
        //return 0;
        return 1;
    }
    else
    {
        glUseProgramObjectARB(this->programObj);
        //this->UpdateParameterHandles();
        return 1;
    }
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::BeginPass(int pass)
{
    n_gltrace("nGLSLShader::BeginPass().");
    glCallList(this->technique[activeTechniqueIdx].pass[pass]);
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::CommitChanges()
{
}

//------------------------------------------------------------------------------
/**
*/
void
nGLSLShader::EndPass()
{
}

//------------------------------------------------------------------------------
/**
stop shader rendering
*/
void
nGLSLShader::End()
{
    if (!this->didNotValidate)
    {
        glUseProgramObjectARB(0);
        n_gltrace("nGLSLShader::End().");
    }
}

//------------------------------------------------------------------------------
/**
*/
bool
nGLSLShader::HasTechnique(const char* t) const
{
    //n_assert(t);
    //n_assert(this->effect);
    //D3DXHANDLE h = this->effect->GetTechniqueByName(t);
    //return (0 != h);

    // TODO: add checking technique
    return -1 != this->techniqueName.FindIndex(t);
}

//------------------------------------------------------------------------------
/**
*/
bool
nGLSLShader::SetTechnique(const char* t)
{
    activeTechniqueIdx = this->techniqueName.FindIndex(t);
    //n_assert(t);
    //n_assert(this->effect);
    //
    //// get handle to technique
    //D3DXHANDLE hTechnique = this->effect->GetTechniqueByName(t);
    //if (0 == hTechnique)
    //{
    //    n_error("nD3D9Shader::SetTechnique(%s): technique not found in shader file %s!\n", t, this->GetFilename());
    //    return false;
    //}

    //// check if technique needs software vertex processing (this is the
    //// case if the 3d device is a mixed vertex processing device, and 
    //// the current technique includes a vertex shader
    //this->curTechniqueNeedsSoftwareVertexProcessing = false;
    //if (nGfxServer2::Instance()->AreVertexShadersEmulated())
    //{
    //    D3DXHANDLE hPass = this->effect->GetPass(hTechnique, 0);
    //    n_assert(0 != hPass);
    //    D3DXPASS_DESC passDesc = { 0 };
    //    HRESULT hr = this->effect->GetPassDesc(hPass, &passDesc);
    //    n_assert(SUCCEEDED(hr));
    //    if (passDesc.pVertexShaderFunction)
    //    {
    //        this->curTechniqueNeedsSoftwareVertexProcessing = true;
    //    }
    //}

    //// finally, set the technique
    //HRESULT hr = this->effect->SetTechnique(hTechnique);
    //if (FAILED(hr))
    //{
    //    n_printf("nD3D9Shader::SetTechnique(%s) on shader %s failed!\n", t, this->GetFilename());
    //    return false;
    //}

    //return true;

    // TODO: add setting technique
    return -1 != activeTechniqueIdx;
}

//------------------------------------------------------------------------------
/**
*/
const char*
nGLSLShader::GetTechnique() const
{
    //n_assert(this->effect);
    //return this->effect->GetCurrentTechnique();

    // TODO: add getting technique
    if (-1 == activeTechniqueIdx) return NULL;
    return this->techniqueName[activeTechniqueIdx].Get();
}

//------------------------------------------------------------------------------
/**
This method is called when the gl device is lost.
*/
void
nGLSLShader::OnLost()
{
    n_assert(Lost != this->GetState());
    //n_assert(this->effect);
    //this->effect->OnLostDevice();
    this->SetState(Lost);

    // flush my current parameters (important! otherwise, seemingly redundant
    // state will not be set after OnRestore())!
    this->curParams.Clear();
}

//------------------------------------------------------------------------------
/**
This method is called when the gl device has been restored.
*/
void
nGLSLShader::OnRestored()
{
    n_assert(Lost == this->GetState());
    //n_assert(this->effect);
    //this->effect->OnResetDevice();
    this->SetState(Valid);
}
