#ifndef PHYSICS_COMPOSITE_H
#define PHYSICS_COMPOSITE_H
//------------------------------------------------------------------------------
/**
    @class Physics::Composite

    A Physics::Composite contains one or several rigid bodies, connected
    by joints. There is one master body defined in the pool which receives
    and delivers positions and forces from/to the outside world. Composites
    are usually constructed once and not altered (i.e. you cannot remove
    bodies from the composite). The master body will always be the body at index
    0 (the first body added to the group).

    Composites may also contain optional static shapes. These are pure collide
    shapes which will move with the composite but will not act physically.

    (C) 2003 RadonLabs GmbH
*/
#include "foundation/refcounted.h"
#include "foundation/ptr.h"
#include "util/narray.h"
#include "util/nstring.h"
#include "physics/rigidbody.h"

//------------------------------------------------------------------------------
namespace Physics
{
class Joint;
class Entity;

class Composite : public Foundation::RefCounted
{
    DeclareRtti;
	DeclareFactory(Composite);

public:
    /// constructor
    Composite();
    /// destructor
    virtual ~Composite();
    /// attach the composite to the world
    virtual void Attach(dWorldID worldID, dSpaceID dynamicSpaceId, dSpaceID staticSpaceId);
    /// detach the composite from the world
    virtual void Detach();
    /// return true if currently attached to world
    bool IsAttached() const;
    /// set position and orientation.
    void SetTransform(const matrix44& m);
    /// get position and orientation.
    const matrix44& GetTransform() const;
    /// return true if transformation has changed during frame
    bool HasTransformChanged() const;
    /// begin adding rigid bodies to the composite
    virtual void BeginBodies(int num);
    /// add a rigid body to the composite (incs refcount)
    virtual void AddBody(RigidBody* body);
    /// finish adding rigid bodies to the composite
    virtual void EndBodies();
    /// get number of rigid bodies in the composite
    int GetNumBodies() const;
    /// get pointer to rigid body at index
    RigidBody* GetBodyAt(int index) const;
    /// find rigid body by its unique id
    RigidBody* FindBodyByUniqueId(int id) const;
    /// does body with name `name' exist?
    bool HasBodyWithName(const nString& name) const;
    /// return true if any of the rigid bodies has the provided visual link type
    bool HasLinkType(RigidBody::LinkType t);
    /// Body with name `name'.
    RigidBody* GetBodyByName(const nString& name) const;
    /// begin adding joints to the composite
    virtual void BeginJoints(int num);
    /// add a joint to the composite
    virtual void AddJoint(Joint* joint);
    /// finish adding joints to the composite
    virtual void EndJoints();
    /// get number of joints in composite
    int GetNumJoints() const;
    /// get pointer to joint at index
    Joint* GetJointAt(int index) const;
    /// begin adding static shapes
    virtual void BeginShapes(int num);
    /// add a static shape to the composite
    virtual void AddShape(Shape* shape);
    /// finish adding shapes
    virtual void EndShapes();
    /// get number of shapes in composite
    int GetNumShapes() const;
    /// get shape at index
    Shape* GetShapeAt(int index) const;
    /// get pointer to the master body (first body added)
    RigidBody* GetMasterBody() const;
    /// called before simulation step is taken
    void OnStepBefore();
    /// called after simulation step is taken
    void OnStepAfter();
    /// called before physics frame is taken
    void OnFrameBefore();
    /// called after simulation frame is taken
    void OnFrameAfter();
    /// enable/disable the composite
    void SetEnabled(bool b);
    /// get enabled state of the composite
    bool IsEnabled() const;
    /// Number of collision in last simulation step
    int GetNumCollisions() const;
    /// Is composite horizontal collided?
    bool IsHorizontalCollided() const;
    /// set the entity we are attached to
    void SetEntity(Entity* e);
    /// get the entity we are attached to
    Entity* GetEntity() const;
    /// reset the velocity and force accumulators of the rigid bodies
    void Reset();
    /// render the debug visualization
    void RenderDebug();

protected:
    /// clear rigid bodies
    void ClearBodies();
    /// clear joints
    void ClearJoints();
    /// clear shapes
    void ClearShapes();

    matrix44 frameBeforeTransform;
    matrix44 transform;
    bool transformChanged;
    bool transformWasSet;
    nFixedArray<Ptr<RigidBody> > bodyArray;
    nFixedArray<Ptr<Joint> > jointArray;
    nFixedArray<Ptr<Shape> > shapeArray;
    int curBodyIndex;
    int curJointIndex;
    int curShapeIndex;
    Entity* entity;
    bool inBeginBodies;
    bool inBeginJoints;
    bool inBeginShapes;
    bool isAttached;
    dSpaceID odeSpaceId;
};

RegisterFactory(Composite);

//------------------------------------------------------------------------------
/**
*/
inline
RigidBody*
Composite::GetMasterBody() const
{
    if (this->bodyArray.Size() > 0)
    {
        return this->bodyArray[0];
    }
    else
    {
        return 0;
    }
}

//------------------------------------------------------------------------------
/**
*/
inline
const matrix44&
Composite::GetTransform() const
{
    return this->transform;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
Composite::IsAttached() const
{
    return this->isAttached;
}

//------------------------------------------------------------------------------
/**
*/
inline
Entity*
Composite::GetEntity() const
{
    return this->entity;
}

//------------------------------------------------------------------------------
/**
    Returns number of bodies in composite.
*/
inline
int
Composite::GetNumBodies() const
{
    return this->bodyArray.Size();
}

//------------------------------------------------------------------------------
/**
    Returns pointer to rigid body at index.
*/
inline
RigidBody*
Composite::GetBodyAt(int index) const
{
    return this->bodyArray[index];
}

//------------------------------------------------------------------------------
/**
    Returns number of joints in composite.
*/
inline
int
Composite::GetNumJoints() const
{
    return this->jointArray.Size();
}

//------------------------------------------------------------------------------
/**
    Return pointer to joint at index.
*/
inline
Joint*
Composite::GetJointAt(int index) const
{
    return this->jointArray[index];
}

//------------------------------------------------------------------------------
/**
    Returns number of shapes in composite.
*/
inline
int
Composite::GetNumShapes() const
{
    return this->shapeArray.Size();
}

//------------------------------------------------------------------------------
/**
    Return pointer to shape at index.
*/
inline
Shape*
Composite::GetShapeAt(int index) const
{
    return this->shapeArray[index];
}

//------------------------------------------------------------------------------
/**
    Return true if transformation has changed during frame.
*/
inline
bool
Composite::HasTransformChanged() const
{
    return this->transformChanged;
}

} // namespace Physics
//------------------------------------------------------------------------------
#endif
