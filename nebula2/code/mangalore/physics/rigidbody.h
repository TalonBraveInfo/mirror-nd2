#ifndef PHYSICS_RIGIDBODY_H
#define PHYSICS_RIGIDBODY_H
//------------------------------------------------------------------------------
/**
    @class Physics::RigidBody

    RigidBody is the internal base class for all types of rigid bodies.
    Subclasses of RigidBody implement specific shapes. RigidBodies can
    be connected by Joints to form a hierarchy.

    (C) 2003 RadonLabs GmbH
*/
#include "foundation/refcounted.h"
#include "mathlib/matrix.h"
#include "mathlib/vector.h"
#include "mathlib/plane.h"
#include "physics/materialtable.h"
#include "physics/server.h"
#include "ode/ode.h"
#include "util/nfixedarray.h"

//------------------------------------------------------------------------------
namespace Physics
{
class Shape;
class Composite;

class RigidBody : public Foundation::RefCounted
{
    DeclareRtti;
	DeclareFactory(RigidBody);

public:
    /// link types (for linking to graphical representation)
    enum LinkType
    {
        ModelNode = 0,
        ShadowNode,
        JointNode,

        NumLinkTypes,
    };

    /// an unique rigid body id
    typedef unsigned int Id;
    /// constructor
    RigidBody();
    /// destructor
    virtual ~RigidBody();
    /// get the rigid body's unique id
    Id GetUniqueId() const;
    /// attach the rigid body to the world and initialize its position
    void Attach(dWorldID worldID, dSpaceID spaceID, const matrix44& m);
    /// detach the rigid body from the world
    void Detach();
    /// return true if currently attached
    bool IsAttached() const;
    /// set name of rigid body
    void SetName(const nString& n);
    /// get name of rigid body
    const nString& GetName() const;
    /// set the body's initial local transform within the composite
    void SetInitialTransform(const matrix44& m);
    /// get the body's initial local transform within the composite
    const matrix44& GetInitialTransform() const;
    /// get the inverse of the initial local transform
    const matrix44& GetInverseInitialTransform() const;
    /// enable/disable the rigid body
    void SetEnabled(bool b);
    /// get enabled/disabled state of the rigid body
    bool IsEnabled() const;
    /// define a link name
    void SetLinkName(LinkType type, const nString& n);
    /// get a link name
    const nString& GetLinkName(LinkType type) const;
    /// return true if a given link type is valid
    bool IsLinkValid(LinkType type) const;
    /// set a link index
    void SetLinkIndex(int index);
    /// get a link index
    int GetLinkIndex() const;
    /// begin adding shapes to the rigid body
    void BeginShapes(int num);
    /// add a shape to the rigid body
    void AddShape(Shape* shape);
    /// finish adding shapes
    void EndShapes();
    /// get number of shapes
    int GetNumShapes() const;
    /// get pointer to shape at index
    Shape* GetShapeAt(int index) const;
    /// set the body's world space transform
    void SetTransform(const matrix44& m);
    /// get the body's world space transform
    const matrix44& GetTransform() const;
    /// set the body's linear velocity
    void SetLinearVelocity(const vector3& v);
    /// get the body's linear velocity
    vector3 GetLinearVelocity() const;
    /// set the body's angular velocity
    void SetAngularVelocity(const vector3& v);
    /// get the body's angular velocity
    vector3 GetAngularVelocity() const;
    /// get the mass of the body
    float GetMass() const;
    /// reset the force and torque accumulators
    void Reset();
    /// transform a global point into the body's local space
    vector3 GlobalToLocalPoint(const vector3& p) const;
    /// transform a body-local point into global space
    vector3 LocalToGlobalPoint(const vector3& p) const;
    /// get the current accumulated force vector in local space
    void GetLocalForce(vector3& v) const;
    /// get the current accumulated torque vector in local space
    void GetLocalTorque(vector3& v) const;
    /// apply a global impulse vector at the next time step at a global position
    void ApplyImpulseAtPos(const vector3& impulse, const vector3& pos);
    /// called before simulation step is taken
    void OnStepBefore();
    /// called after simulation step is taken
    void OnStepAfter();
    /// called before simulation takes place
    void OnFrameBefore();
    /// called after simulation takes place
    void OnFrameAfter();
    /// Number of collisions in last simulation step
    int GetNumCollisions() const;
    /// Is rigid body horizontal collided?
    bool IsHorizontalCollided() const;
    /// set pointer to composite we are attached to
    void SetComposite(Composite* c);
    /// get pointer to composite we are attached to
    Composite* GetComposite() const;
    /// set pointer to physics entity we are attached to
    void SetEntity(Entity* e);
    /// get pointer to entity we are attached to
    Entity* GetEntity() const;
    /// render the debug shapes
    void RenderDebug();
    /// get the ode rigid body id
    dBodyID GetOdeBodyId() const;
    /// enable/disable collision for connected bodies
    void SetConnectedCollision(bool b);
    /// get connected collision flag
    bool GetConnectedCollision() const;
    /// set angular damping factor (0.0f..1.0f)
    void SetAngularDamping(float f);
    /// get angular damping factor (0.0f..1.0f)
    float GetAngularDamping() const;
    /// set linear damping factor
    void SetLinearDamping(float f);
    /// get linear damping factor
    float GetLinearDamping() const;
    /// set temporary stamp value
    void SetStamp(uint s);
    /// get temporary stamp value
    uint GetStamp() const;
    /// set whether body is influenced by gravity
    void SetEnableGravity(bool enable);
    /// get whether body is influenced by gravity
    bool GetEnableGravity() const;

private:
    /// add ode mass structure to internal mass
    void AddShapeMass(const matrix44& m, dMass& shapeMass);
    /// apply angular damping
    void ApplyDamping();

    friend class Shape;
    friend class Joint;

    static Id uniqueIdCounter;

    matrix44 initialTransform;
    matrix44 inverseInitialTransform;
    matrix44 transform;
    Id uniqueId;
    nString name;
    nFixedArray<nString> linkNames;
    int linkIndex;
    Composite* composite;
    Entity* entity;
    dBodyID odeBodyId;
    dMass mass;                         // the mass structure of the rigid body
    int curShapeIndex;
    nFixedArray<Ptr<Shape> > shapeArray;
    bool connectedCollision;
    float angularDamping;
    float linearDamping;
    uint stamp;
};

RegisterFactory(RigidBody);

//------------------------------------------------------------------------------
/**
*/
inline
void
RigidBody::SetStamp(uint s)
{
    this->stamp = s;
}

//------------------------------------------------------------------------------
/**
*/
inline
uint
RigidBody::GetStamp() const
{
    return this->stamp;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
RigidBody::IsAttached() const
{
    return (0 != this->odeBodyId);
}

//------------------------------------------------------------------------------
/**
*/
inline
int
RigidBody::GetNumShapes() const
{
    return this->shapeArray.Size();
}

//------------------------------------------------------------------------------
/**
*/
inline
Shape*
RigidBody::GetShapeAt(int index) const
{
    return this->shapeArray[index];
}

//------------------------------------------------------------------------------
/**
*/
inline
RigidBody::Id
RigidBody::GetUniqueId() const
{
    return this->uniqueId;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
RigidBody::SetComposite(Composite* c)
{
    this->composite = c;
}

//------------------------------------------------------------------------------
/**
*/
inline
Composite*
RigidBody::GetComposite() const
{
    return this->composite;
}

//------------------------------------------------------------------------------
/**
*/
inline
Entity*
RigidBody::GetEntity() const
{
    return this->entity;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
RigidBody::SetName(const nString& n)
{
    this->name = n;
}

//------------------------------------------------------------------------------
/**
*/
inline
const nString&
RigidBody::GetName() const
{
    return this->name;
}

//------------------------------------------------------------------------------
/**
*/
inline
dBodyID
RigidBody::GetOdeBodyId() const
{
    return this->odeBodyId;
}

//------------------------------------------------------------------------------
/**
    Enable/disable the rigid body. Bodies which have reached a resting position
    should be disabled. Bodies will enable themselves as soon as they are
    touched by another body.

    @param  b   new enabled state
*/
inline
void
RigidBody::SetEnabled(bool b)
{
    if (b) dBodyEnable(this->odeBodyId);
    else   dBodyDisable(this->odeBodyId);
}

//------------------------------------------------------------------------------
/**
    Returns the current enabled state of the body.

    @return     true if body is enabled
*/
inline
bool
RigidBody::IsEnabled() const
{
    return (dBodyIsEnabled(this->odeBodyId) > 0);
}

//------------------------------------------------------------------------------
/**
    Get the body's mass.

    @return     mass
*/
inline
float
RigidBody::GetMass() const
{
    return this->mass.mass;
}

//------------------------------------------------------------------------------
/**
    Reset the force and torque accumulators of the body.
*/
inline
void
RigidBody::Reset()
{
    dBodySetForce(this->odeBodyId, 0.0f, 0.0f, 0.0f);
    dBodySetTorque(this->odeBodyId, 0.0f, 0.0f, 0.0f);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
RigidBody::SetLinkName(LinkType type, const nString& s)
{
    this->linkNames[type] = s;
}

//------------------------------------------------------------------------------
/**
*/
inline
const nString&
RigidBody::GetLinkName(LinkType type) const
{
    return this->linkNames[type];
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
RigidBody::IsLinkValid(LinkType type) const
{
    return this->linkNames[type].IsValid();
}

//------------------------------------------------------------------------------
/**
*/
inline
void
RigidBody::SetLinkIndex(int i)
{
    this->linkIndex = i;
}

//------------------------------------------------------------------------------
/**
*/
inline
int
RigidBody::GetLinkIndex() const
{
    return this->linkIndex;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
RigidBody::SetInitialTransform(const matrix44& m)
{
    this->initialTransform = m;
    this->inverseInitialTransform = m;
    this->inverseInitialTransform.invert();
}

//------------------------------------------------------------------------------
/**
*/
inline
const matrix44&
RigidBody::GetInitialTransform() const
{
    return this->initialTransform;
}

//------------------------------------------------------------------------------
/**
*/
inline
const matrix44&
RigidBody::GetInverseInitialTransform() const
{
    return  this->inverseInitialTransform;
}

//------------------------------------------------------------------------------
/**
    Enable/disable collision for bodies which are connected by a joint.
    For collision, both bodies connected by a joint must have this flag set.
    Default is off.
*/
inline
void
RigidBody::SetConnectedCollision(bool b)
{
    this->connectedCollision = b;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
RigidBody::GetConnectedCollision() const
{
    return this->connectedCollision;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
RigidBody::SetAngularDamping(float f)
{
    n_assert((f >= 0.0f) && (f <= 1.0f));
    this->angularDamping = f;
}

//------------------------------------------------------------------------------
/**
*/
inline
float
RigidBody::GetAngularDamping() const
{
    return this->angularDamping;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
RigidBody::SetLinearDamping(float f)
{
    n_assert((f >= 0.0f) && (f <= 1.0f));
    this->linearDamping = f;
}

//------------------------------------------------------------------------------
/**
*/
inline
float
RigidBody::GetLinearDamping() const
{
    return this->linearDamping;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
RigidBody::SetEnableGravity(bool enable)
{
    dBodySetGravityMode(this->odeBodyId, enable ? 1 : 0);
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
RigidBody::GetEnableGravity() const
{
    return dBodyGetGravityMode(this->odeBodyId) != 0;
}

} // namespace Physics
//------------------------------------------------------------------------------
#endif
