#ifndef MSG_GFXADDATTACHMENT_H
#define MSG_GFXADDATTACHMENT_H
//------------------------------------------------------------------------------
/**
    @class Message::GfxAddAttachment

    Attach a graphics entity defined by a resource name to a joint.

    (C) 2005 Radon Labs GmbH
*/
#include "message/msg.h"
#include "graphics/entity.h"

//------------------------------------------------------------------------------
namespace Message
{
class GfxAddAttachment : public Msg
{
    DeclareRtti;
    DeclareFactory(GfxAddAttachment);
    DeclareMsgId;

public:
    /// constructor
    GfxAddAttachment();

    /// set joint name
    void SetJointName(const nString& n);
    /// get joint name
    const nString& GetJointName() const;
    /// set graphics resource name
    void SetResourceName(const nString& n);
    /// get graphics resource name
    const nString& GetResourceName() const;
    /// set as alternative the graphics entity
    void SetGraphicsEntity(Graphics::Entity* gfxEntity);
    /// get the alternative graphics entity
    Graphics::Entity* GetGraphicsEntity();
    /// set optional offset matrix
    void SetOffsetMatrix(const matrix44& m);
    /// get offset matrix
    const matrix44& GetOffsetMatrix() const;

private:
    nString jointName;
    nString resName;
    Graphics::Entity* gfxEntity;
    matrix44 offsetMatrix;
};

RegisterFactory(GfxAddAttachment);

//------------------------------------------------------------------------------
/**
*/
inline
void
GfxAddAttachment::SetJointName(const nString& n)
{
    this->jointName = n;
}

//------------------------------------------------------------------------------
/**
*/
inline
const nString&
GfxAddAttachment::GetJointName() const
{
    return this->jointName;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
GfxAddAttachment::SetResourceName(const nString& n)
{
    this->resName = n;
}

//------------------------------------------------------------------------------
/**
*/
inline
const nString&
GfxAddAttachment::GetResourceName() const
{
    return this->resName;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
GfxAddAttachment::SetGraphicsEntity(Graphics::Entity* gfxEntity)
{
    this->gfxEntity = gfxEntity;
}

//------------------------------------------------------------------------------
/**
*/
inline
Graphics::Entity*
GfxAddAttachment::GetGraphicsEntity()
{
    return this->gfxEntity;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
GfxAddAttachment::SetOffsetMatrix(const matrix44& m)
{
    this->offsetMatrix = m;
}

//------------------------------------------------------------------------------
/**
*/
inline
const matrix44&
GfxAddAttachment::GetOffsetMatrix() const
{
    return this->offsetMatrix;
}

} // namespace Message
//------------------------------------------------------------------------------
#endif
