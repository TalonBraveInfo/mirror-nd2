#ifndef PROPERTIES_VIDEOCAMERAPROPERTY_H
#define PROPERTIES_VIDEOCAMERAPROPERTY_H
//------------------------------------------------------------------------------
/**
    @class Properties::VideoCameraProperty

    A manually controlled camera property which implements different
    control models.

    (C) 2005 Radon Labs GmbH
*/
#include "properties/cameraproperty.h"
#include "tools/nmayacamcontrol.h"

//------------------------------------------------------------------------------
namespace Attr
{
    // video camera specific attributes
    DeclareVector3(VideoCameraCenterOfInterest);
    DeclareVector3(VideoCameraDefaultUpVec);
};
//------------------------------------------------------------------------------
namespace Properties
{
class VideoCameraProperty : public CameraProperty
{
    DeclareRtti;
	DeclareFactory(VideoCameraProperty);

public:
    /// constructor
    VideoCameraProperty();
    /// destructor
    virtual ~VideoCameraProperty();
    /// called from Entity::ActivateProperties()
    virtual void OnActivate();
    /// called when camera focus is obtained
    virtual void OnObtainFocus();
    /// called before rendering happens
    virtual void OnRender();
    /// setup default entity attributes
    virtual void SetupDefaultAttributes();
    /// return true if message is accepted by a property
    virtual bool Accepts(Message::Msg* msg);
    /// handle a single message
    virtual void HandleMessage(Message::Msg* msg);
private:
    nMayaCamControl mayaCamera;
};

RegisterFactory(VideoCameraProperty);

} // namespace Properties
//------------------------------------------------------------------------------
#endif
