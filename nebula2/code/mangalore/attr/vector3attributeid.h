#ifndef ATTR_VECTOR3_ATTRIBUTE_ID_H
#define ATTR_VECTOR3_ATTRIBUTE_ID_H
//------------------------------------------------------------------------------
/**
    @class Vector3AttributeID

    @brief see class AttributeID

    (C) 2005 Radon Labs GmbH

*/
#include "attr/attributeid.h"

namespace Attr
{

class Vector3AttributeID : public AttributeID
{
public:
    Vector3AttributeID();
    Vector3AttributeID(const _attridTyped<attr::Vector3T>*);
    Vector3AttributeID(const nString& name); // must exist!
    virtual ~Vector3AttributeID();

    /// equality operator
    friend bool operator==(const Vector3AttributeID& lhs, const Vector3AttributeID& rhs);
    /// inequality operator
    friend bool operator!=(const Vector3AttributeID& lhs, const Vector3AttributeID& rhs);

    static Vector3AttributeID FindVector3AttributeID(const nString& name);
};

//------------------------------------------------------------------------------
/**
*/
inline
bool
operator==(const Vector3AttributeID& lhs, const Vector3AttributeID& rhs)
{
    return lhs.attridPtr == rhs.attridPtr;
}

//------------------------------------------------------------------------------
/**
*/
inline
bool
operator!=(const Vector3AttributeID& lhs, const Vector3AttributeID& rhs)
{
    return lhs.attridPtr != rhs.attridPtr;
}

//------------------------------------------------------------------------------
/**
*/
inline
Vector3AttributeID::Vector3AttributeID(const _attridTyped<attr::Vector3T>* aip) :
    AttributeID(aip)
{
}

//------------------------------------------------------------------------------
/**
*/
inline
Vector3AttributeID::Vector3AttributeID() :
    AttributeID()
{
}

//------------------------------------------------------------------------------
/**
    Gives the AttributeID "name".
    Will fail hard if AttributeID doesn't exist.
*/
inline
Vector3AttributeID::Vector3AttributeID(const nString& name)
{
    const Vector3AttributeID& existingID = Vector3AttributeID::FindVector3AttributeID(name);

    if (!existingID.IsValid())
    {
        n_error("Error: Attribute ID of name \"%s\" not found!", name.Get());
    }

    this->attridPtr = existingID.attridPtr;
}

//------------------------------------------------------------------------------
/**
*/
inline
Vector3AttributeID::~Vector3AttributeID()
{
}

} // namespace Attr

//------------------------------------------------------------------------------
#endif
