#ifndef N_SCENENODE_H
#define N_SCENENODE_H
//------------------------------------------------------------------------------
/**
    @class nSceneNode
    @ingroup NebulaSceneSystemNodes

    The nSceneNode is the base class of all objects which can be attached
    to a scene managed by the nSceneServer class. A scene node object
    may provide transform, geometry, shader and volume information.

    See also @ref N2ScriptInterface_nscenenode

    (C) 2002 RadonLabs GmbH
*/
#include "kernel/nroot.h"
#include "util/narray.h"
#include "mathlib/matrix.h"
#include "mathlib/bbox.h"
#include "kernel/ndynautoref.h"
#include "gfx2/nshaderparams.h"

class nSceneServer;
class nRenderContext;
class nGfxServer2;
class nAnimator;
class nVariableServer;

//-------------------------------------------------------------------------------
class nSceneNode : public nRoot
{
public:
    /// constructor
    nSceneNode();
    /// destructor
    virtual ~nSceneNode();
    /// save object to persistent stream
    virtual bool SaveCmds(nPersistServer* ps);
    /// load resources for this object
    virtual bool LoadResources();
    /// unload resources for this object
    virtual void UnloadResources();
    /// return true if resources for this object are valid
    bool AreResourcesValid() const;
    /// recursively preload resources
    void PreloadResources();
    /// called by app when new render context has been created for this object
    virtual void RenderContextCreated(nRenderContext* renderContext);
    /// called by app when render context is going to be released
    virtual void RenderContextDestroyed(nRenderContext* renderContext);
    /// called by nSceneServer when object is attached to scene
    virtual void Attach(nSceneServer* sceneServer, nRenderContext* renderContext);
    /// return true if node provides transformion
    virtual bool HasTransform() const;
    /// return true if node provides geometry
    virtual bool HasGeometry() const;
    /// return true if node provides shader
    virtual bool HasShader(uint fourcc) const;
    /// return true if node provides lighting information
    virtual bool HasLight() const;
    /// render transformation
    virtual bool RenderTransform(nSceneServer* sceneServer, nRenderContext* renderContext, const matrix44& parentMatrix);
    /// render geometry
    virtual bool RenderGeometry(nSceneServer* sceneServer, nRenderContext* renderContext);
    /// render shader
    virtual bool RenderShader(uint fourcc, nSceneServer* sceneServer, nRenderContext* renderContext);
    /// write light volume parameters into the provided shader params object
    virtual bool RenderLight(nSceneServer* sceneServer, nRenderContext* renderContext, const matrix44& lightTransform);
    /// set the local bounding box
    void SetLocalBox(const bbox3& b);
    /// get the node's bounding box
    const bbox3& GetLocalBox() const;
    /// set render priority
    void SetRenderPri(int pri);
    /// get render priority
    int GetRenderPri() const;
    /// add an animator object
    void AddAnimator(const char* path);
    /// get number of animator objects
    int GetNumAnimators() const;
    /// get animator object at index
    const char* GetAnimatorAt(int index);
    /// invoke all shader animators
    void InvokeShaderAnimators(nRenderContext* renderContext);
    /// invoke all transform animators
    void InvokeTransformAnimators(nRenderContext* renderContext);

    static nKernelServer* kernelServer;

protected:
    nAutoRef<nVariableServer> refVariableServer;
    nAutoRef<nSceneServer> refSceneServer;
    bbox3 localBox;
    nArray< nDynAutoRef<nAnimator> > animatorArray;
    bool resourcesValid;
    int renderPri;
};

//------------------------------------------------------------------------------
/**
    Return true if the node's resources are valid.
*/
inline
bool
nSceneNode::AreResourcesValid() const
{
    return this->resourcesValid;
}

//------------------------------------------------------------------------------
/**
    Define the local bounding box. Shape node compute their bounding
    box automatically at load time. This method can be used to define
    bounding boxes for other nodes. This may be useful for higher level
    code like gameframeworks. Nebula itself only uses bounding boxes
    defined on shape nodes.
*/
inline
void
nSceneNode::SetLocalBox(const bbox3& b)
{
    this->localBox = b;
}

//------------------------------------------------------------------------------
/**
*/
inline
const bbox3&
nSceneNode::GetLocalBox() const
{
    return this->localBox;
}

//------------------------------------------------------------------------------
/**
    Set the render priority. This should be a number between -127 and +127,
    the default is 0. Smaller numbers will render first.
*/
inline
void
nSceneNode::SetRenderPri(int pri)
{
    n_assert((pri >= -127) && (pri <= 127));
    this->renderPri = pri;
}

//------------------------------------------------------------------------------
/**
    Get the render priority of this node.
*/
inline
int
nSceneNode::GetRenderPri() const
{
    return this->renderPri;
}

//------------------------------------------------------------------------------
#endif
