//------------------------------------------------------------------------------
//  attr/attributes.cc
//  (C) 2005 Radon Labs GmbH
//------------------------------------------------------------------------------
#include "attr/attributes.h"

namespace Attr
{
    DefineFloat(TestFloatAttr);
    DefineInt(TestIntAttr);
    DefineString(TestStringAttr);
    DefineVector3(TestVector3Attr);
    DefineVector4(TestVector4Attr);
    DefineBool(TestBoolAttr);
    DefineString(_Dummy);
    DefineString(_Type);
    DefineString(_Category);
    DefineStorableString(_Level);
    DefineString(_Layers);
    DefineString(_ID);
    DefineBool(StartLevel);
    DefineBool(RandomEncounterLevel);
    DefineStorableString(GUID);
    DefineVector3(Center);
    DefineVector3(Extents);
    DefineStorableString(CurrentLevel);
    DefineString(Id);
    DefineString(Name);
    DefineString(Placeholder);
    DefineBool(Rot180);
    DefineStorableMatrix44(Transform);
    DefineInt(AnimIndex);
    DefineString(Physics);
    DefineString(Audio);
    DefineBool(Collide);
    DefineBool(AllowScale);
    DefineString(PhysicMaterial);
    DefineFloat(AudioInnerRadius);
    DefineFloat(AudioOuterRadius);
    DefineBool(AudioFadein);
    DefineStorableInt(TargetEntityId);
    DefineStorableFloat(Time);
    DefineString(File);
    DefineStorableString(EntityGroup);

    DefineVector3(VelocityVector);

    DefineFloat(RelVelocity);
    DefineFloat(MaxVelocity);
    DefineBool(Following);
    DefineBool(Moving);

    DefineString(Graphics);

    DefineString(AnimSet);
    DefineString(CharacterSet);
    DefineString(SoundSet);
}
