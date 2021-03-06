#ifndef PHYSICS_SERVER_H
#define PHYSICS_SERVER_H
//------------------------------------------------------------------------------
/**
    @class Physics::Server

    The physics subsystem server object.

    (C) 2003 RadonLabs GmbH
*/
#include "foundation/refcounted.h"
#include "foundation/ptr.h"
#include "physics/materialtable.h"
#include "physics/joint.h"
#include "physics/contactpoint.h"
#include "physics/entity.h"
#include "physics/ray.h"
#include "util/nkeyarray.h"

//------------------------------------------------------------------------------
namespace Physics
{
class Level;
class Composite;
class Ragdoll;
class RigidBody;
class Shape;
class BoxShape;
class SphereShape;
class MeshShape;
class CapsuleShape;
class Ray;
class HingeJoint;
class UniversalJoint;
class SliderJoint;
class BallJoint;
class Hinge2Joint;
class AMotor;
class MouseGripper;
class AreaImpulse;

class Server : public Foundation::RefCounted
{
    DeclareRtti;
	DeclareFactory(Server);

public:
    /// constructor
    Server();
    /// destructor
    virtual ~Server();
    /// get instance pointer
    static Server* Instance();
    /// initialize the physics subsystem
    virtual bool Open();
    /// close the physics subsystem
    virtual void Close();
    /// Is server open?
    bool IsOpen() const;
    /// perform simulation step(s)
    virtual void Trigger();
    /// set the current physics level object
    void SetLevel(Level* level);
    /// get the current physics level object
    Level* GetLevel() const;
    /// access to global embedded mouse gripper
    MouseGripper* GetMouseGripper() const;
    /// set the current simulation time
    void SetTime(nTime t);
    /// get the current simulation time
    nTime GetTime() const;
    /// set current point of interest
    void SetPointOfInterest(const vector3& p);
    /// get current point of interest
    const vector3& GetPointOfInterest() const;
    /// find a registered physics entity by its unique id
    Entity* FindEntityByUniqueId(Entity::Id uniqueId) const;
    /// create a new composite object
    virtual Composite* CreateComposite() const;
    /// create a new ragdoll object
    virtual Ragdoll* CreateRagdoll() const;
    /// create a new rigid body object
    virtual RigidBody* CreateRigidBody() const;
    /// create a new hinge joint object
    virtual HingeJoint* CreateHingeJoint() const;
    /// create a new hinge2 joint object
    virtual Hinge2Joint* CreateHinge2Joint() const;
    /// create a new universal joint object
    virtual UniversalJoint* CreateUniversalJoint() const;
    /// create new slider joint object
    virtual SliderJoint* CreateSliderJoint() const;
    /// create new ball joint object
    virtual BallJoint* CreateBallJoint() const;
    /// create a new amotor object
    virtual AMotor* CreateAMotor() const;
    /// create a box shape object
    virtual BoxShape* CreateBoxShape(const matrix44& m, MaterialType matType, const vector3& size) const;
    /// create a sphere shape object
    virtual SphereShape* CreateSphereShape(const matrix44& m, MaterialType matType, float radius) const;
    /// create a capsule shape object
    virtual CapsuleShape* CreateCapsuleShape(const matrix44& m, MaterialType matType, float radius, float length) const;
    /// create a mesh shape object
    virtual MeshShape* CreateMeshShape(const matrix44& m, MaterialType matType, const nString& meshFilename) const;
    /// create a ray object
    virtual Ray* CreateRay(const vector3& orig, const vector3& vec) const;
    /// Do a ray check starting from position `pos' along ray `dir'.
    bool RayCheck(const vector3& pos, const vector3& dir, const FilterSet& excludeSet);
    /// return contact points generated by RayCheck()
    const nArray<ContactPoint>& GetContactPoints() const;
    /// do a ray check through the mouse pointer and return closest contact
    const ContactPoint* GetClosestContactUnderMouse(const vector2& mousePos, float length, const FilterSet& excludeSet);
    /// get closest contact along ray
    const ContactPoint* GetClosestContactAlongRay(const vector3& pos, const vector3& dir, const FilterSet& excludeSet);
    /// apply an impulse along a ray into the world onto the first object which the ray hits
    bool ApplyImpulseAlongRay(const vector3& pos, const vector3& dir, const FilterSet& excludeSet, float impulse);
    /// apply an area impulse to the world
    void ApplyAreaImpulse(AreaImpulse* impulse);
    /// return all entities within a spherical area
    int GetEntitiesInSphere(const vector3& pos, float radius, const FilterSet& excludeSet, nArray<Ptr<Entity> >& result);
    /// return all entities within a box
    int GetEntitiesInBox(const vector3& scale, const matrix44& m, const FilterSet& excludeSet, nArray<Ptr<Entity> >& result);
    /// render a debug visualization of the level
    virtual void RenderDebug();
    /// convert matrix44 to Ode matrix
    static void Matrix44ToOde(const matrix44& from, dMatrix3& to);
    /// convert Ode matrix to matrix44
    static void OdeToMatrix44(const dMatrix3& from, matrix44& to);
    /// convert vector3 to Ode vector
    static void Vector3ToOde(const vector3& from, dVector3& to);
    /// convert Ode vector to vector3
    static void OdeToVector3(const dVector3& from, vector3& to);
    /// get an unique stamp value
    static uint GetUniqueStamp();
    /// get ode world id
    dWorldID GetOdeWorldId() const;
    /// get ode dynamic collide space id
    dSpaceID GetOdeDynamicSpaceId() const;
    /// get ode static collide space id
    dSpaceID GetOdeStaticSpaceId() const;
    /// get the common collide space id which contains the static and dynamic collide space
    dSpaceID GetOdeCommonSpaceId() const;

protected:
    /// register an entity with the server
    void RegisterEntity(Entity* entity);
    /// unregister an entity from the server
    void UnregisterEntity(Entity* entity);

    friend class Level;
    friend class RigidBody;
    friend class TerrainEntity;
    friend class Shape;
    friend class Ray;
    friend class MouseGripper;
    friend class Entity;

    static Server* Singleton;
    static uint UniqueStamp;
    nArray<ContactPoint> contactPoints;
    Ptr<Level> curLevel;
    nTime time;
    bool isOpen;

    Ray ray;

    nKeyArray<Entity*> entityRegistry;
};

RegisterFactory(Server);

//------------------------------------------------------------------------------
/**
*/
inline
Server*
Server::Instance()
{
    n_assert(0 != Singleton);
    return Singleton;
}

//------------------------------------------------------------------------------
/**
*/
inline
uint
Server::GetUniqueStamp()
{
    return ++UniqueStamp;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
Server::IsOpen() const
{
    return isOpen;
}

//------------------------------------------------------------------------------
/**
    Set the current time. Should be called before each call to Trigger().

    @param  t   the current time in seconds
*/
inline
void
Server::SetTime(nTime t)
{
    this->time = t;
}

//------------------------------------------------------------------------------
/**
    Get the current time.

    @return     current time
*/
inline
nTime
Server::GetTime() const
{
    return this->time;
}

//------------------------------------------------------------------------------
/**
*/
inline
const nArray<ContactPoint>&
Server::GetContactPoints() const
{
    return this->contactPoints;
}

//------------------------------------------------------------------------------
/**
    Converts the rotational part of a matrix44 to ODE. This includes
    a handedness conversion (Mangalore is right handed, ODE left handed(?)).
*/
inline
void
Server::Matrix44ToOde(const matrix44& from, dMatrix3& to)
{
    // write the transpose of the original matrix
    to[0]=from.M11; to[1]=from.M21; to[2]=from.M31; to[3]=0.0f;
    to[4]=from.M12; to[5]=from.M22; to[6]=from.M32; to[7]=0.0f;
    to[8]=from.M13; to[9]=from.M23; to[10]=from.M33; to[11]=0.0f;
}

//------------------------------------------------------------------------------
/**
    Convert a dMatrix3 to the rotational part of a matrix44. Includes
    handedness conversion.
*/
inline
void
Server::OdeToMatrix44(const dMatrix3& from, matrix44& to)
{
    to.M11=from[0]; to.M12=from[4]; to.M13=from[8];
    to.M21=from[1]; to.M22=from[5]; to.M23=from[9];
    to.M31=from[2]; to.M32=from[6]; to.M33=from[10];
}

//------------------------------------------------------------------------------
/**
    Convert a vector3 to an Ode vector.
*/
inline
void
Server::Vector3ToOde(const vector3& from, dVector3& to)
{
    to[0]=from.x; to[1]=from.y; to[2]=from.z; to[3]=1.0f;
}

//------------------------------------------------------------------------------
/**
    Convert an Ode vector to a vector3.
*/
inline
void
Server::OdeToVector3(const dVector3& from, vector3& to)
{
    to.set(from[0], from[1], from[2]);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
Server::RegisterEntity(Entity* entity)
{
    n_assert(entity)
    this->entityRegistry.Add(entity->GetUniqueId(), entity);
}

//------------------------------------------------------------------------------
/**
*/
inline
void
Server::UnregisterEntity(Entity* entity)
{
    n_assert(entity);
    this->entityRegistry.Rem(entity->GetUniqueId());
}

//------------------------------------------------------------------------------
/**
    Find a physics entity by its unique id. Can return 0!

    @param  uniqueId    unique id of entity to find
    @return             pointer to entity, or 0 if entity doesn't exist
*/
inline
Entity*
Server::FindEntityByUniqueId(Entity::Id uniqueId) const
{
    Entity* entity = 0;
    if (0 != uniqueId)
    {
        this->entityRegistry.Find(uniqueId, entity);
    }
    return entity;
}

} // namespace Physics
//------------------------------------------------------------------------------
#endif
