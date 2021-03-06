//------------------------------------------------------------------------------
//  nfont2_main.cc
//  (C) 2003 RadonLabs GmbH
//------------------------------------------------------------------------------
#include "gfx2/nfont2.h"

nNebulaClass(nFont2, "nresource");

//------------------------------------------------------------------------------
/**
*/
nFont2::nFont2()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
nFont2::~nFont2()
{
    if (!this->IsUnloaded())
    {
        this->Unload();
    }
}

//------------------------------------------------------------------------------
/**
*/
void
nFont2::SetFontDesc(const nFontDesc& desc)
{
    this->fontDesc = desc;
}

//------------------------------------------------------------------------------
/**
*/
const nFontDesc&
nFont2::GetFontDesc() const
{
    return this->fontDesc;
}
