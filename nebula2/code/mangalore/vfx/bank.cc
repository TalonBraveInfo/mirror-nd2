//------------------------------------------------------------------------------
//  vfx/bank.cc
//  (C) 2003 RadonLabs GmbH
//------------------------------------------------------------------------------
#include "vfx/bank.h"

namespace VFX
{
ImplementRtti(VFX::Bank, Foundation::RefCounted);
ImplementFactory(VFX::Bank);

//------------------------------------------------------------------------------
/**
*/
Bank::Bank()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
Bank::~Bank()
{
    // empty
}

//------------------------------------------------------------------------------
/**
    Open an effect xls table and parse it.

    @return         true if succeeded, false if not
*/
bool
Bank::Open()
{
    if (!this->xmlSpreadSheet.Open())
    {
        n_error("VFX::Bank::Open(): failed to load '%s'", this->xmlSpreadSheet.GetFilename().Get());
        return false;
    }

    nXmlTable& xmlTable = this->xmlSpreadSheet.TableAt(0);
    int numRows = xmlTable.NumRows();
    n_assert(numRows >= 1);
    int numSounds = numRows - 1;

    int effectIndex;
    for (effectIndex = 0; effectIndex < numSounds; effectIndex++)
    {
        int rowIndex = effectIndex + 1;

        // create a new effect resource
        const nString& name = xmlTable.Cell(rowIndex, "Name").AsString();
        const nString& gfxResource = xmlTable.Cell(rowIndex, "File").AsString();
        float timeout = xmlTable.Cell(rowIndex, "Timeout").AsFloat();
        float hotSpot = 0.0f;
        if (xmlTable.HasColumn("Hotspot"))
        {
            hotSpot = xmlTable.Cell(rowIndex, "Hotspot").AsFloat();
        }
        this->AddGraphicsEffect(name, gfxResource, timeout, hotSpot);
    }
    return true;
}

//------------------------------------------------------------------------------
/**
    Close the audio xls table.
*/
void
Bank::Close()
{
    this->xmlSpreadSheet.Close();
}

//------------------------------------------------------------------------------
/**
    Find an effect template by name.

    @param  name      name of effect to find
*/
GraphicsEffect*
Bank::FindEffect(const nString& name)
{
    n_assert(name.IsValid());
    int num = this->effectArray.Size();
    for (int i = 0; i < num; i++)
    {
        if (name == this->effectArray[i]->GetName())
        {
            return this->effectArray[i];
        }
    }
    // fall through: effect template doesn't exist
    return 0;
}

//------------------------------------------------------------------------------
/**
    Add a new effect template to the effect bank.

    @param  effectName      name by which the effect is identified
    @param  resourceName    graphics resource name
    @param  duration        duration of the effect
    @param  hotspot         hotspot time of the effect
*/
void
Bank::AddGraphicsEffect(const nString& effectName, const nString& resourceName, nTime duration, nTime hotspot)
{
    n_assert(effectName.IsValid());
    n_assert(duration > 0.0);

    // make sure the effect doesn't exist yet
    n_assert(!this->FindEffect(effectName));

    // create a new effect object and add it to effect array
    if (resourceName.IsValid())
    {
        GraphicsEffect* newEffect = GraphicsEffect::Create();
        newEffect->SetName(effectName);
        newEffect->SetResourceName(resourceName);
        newEffect->SetDuration(duration);
        newEffect->SetHotspotTime(hotspot);
        this->effectArray.Append(newEffect);
    }
}

//------------------------------------------------------------------------------
/**
    Create a new effect as a clone of an existing effect template, and
    return a pointer to it.

    @param  templateEffectName      name of effect to clone from
    @param  transform               the world position at which the effect should be created
*/
GraphicsEffect*
Bank::CreateGraphicsEffect(const nString& templateEffectName, const matrix44& transform)
{
    n_assert(templateEffectName.IsValid());
    GraphicsEffect* templateEffect = this->FindEffect(templateEffectName);
    if (templateEffect)
    {
        GraphicsEffect* newEffect = GraphicsEffect::Create();
        newEffect->SetName(templateEffect->GetName());
        newEffect->SetDuration(templateEffect->GetDuration());
        newEffect->SetHotspotTime(templateEffect->GetHotspotTime());
        newEffect->SetResourceName(templateEffect->GetResourceName());
        newEffect->SetTransform(transform);
        return newEffect;
    }
    else
    {
        return 0;
    }
}

} // namespace VFX
