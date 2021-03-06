#ifndef MSG_MOVEDIRECTION_H
#define MSG_MOVEDIRECTION_H
//------------------------------------------------------------------------------
/**
    @class Message::MoveDirection

    A MoveDirection message. Expected behavior is that the entity
    starts to move into the specified direction. The direction vector
    can be defined as camera relative or absolute. The velocity settings
    should be interpreted as a factor.

    (C) 2005 Radon Labs GmbH
*/
#include "message/msg.h"

//------------------------------------------------------------------------------
namespace Message
{
class MoveDirection : public Msg
{
    DeclareRtti;
    DeclareFactory(MoveDirection);
    DeclareMsgId;

public:
    /// constructor
    MoveDirection();
    /// set the direction vector
    void SetDirection(const vector3& v);
    /// get the direction vector
    const vector3& GetDirection() const;
    /// set correspond pose
    void SetCorrespondPose(bool b);
    /// get correspond pose
    bool GetCorrespondPose() const;
    /// set camera relative flag
    void SetCameraRelative(bool b);
    /// get camera relative flag
    bool GetCameraRelative() const;

private:
    vector3 dir;
    bool correspondPose;
    bool camRelative;
};

RegisterFactory(MoveDirection);

//------------------------------------------------------------------------------
/**
*/
inline
MoveDirection::MoveDirection() :
    dir(1.0f, 0.0f, 0.0f),
    correspondPose(true),
    camRelative(false)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
inline
void
MoveDirection::SetDirection(const vector3& v)
{
    this->dir = v;
}

//------------------------------------------------------------------------------
/**
*/
inline
const vector3&
MoveDirection::GetDirection() const
{
    return this->dir;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
MoveDirection::SetCorrespondPose(bool b)
{
    this->correspondPose = b;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
MoveDirection::GetCorrespondPose() const
{
    return this->correspondPose;
}

//------------------------------------------------------------------------------
/**
*/
inline
void
MoveDirection::SetCameraRelative(bool b)
{
    this->camRelative = b;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
MoveDirection::GetCameraRelative() const
{
    return this->camRelative;
}

} // namespace Message
//------------------------------------------------------------------------------
#endif
