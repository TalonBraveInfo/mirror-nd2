//-------------------------------------------------------------------
//  nspatialsector_cmds.cc
//  (C) 2004 Gary Haussmann
//-------------------------------------------------------------------
#include "spatialdb/nspatialsector.h"

//------------------------------------------------------------------------------
/**
    @scriptclass
    nspatialsector

    @superclass
    nroot

    @classinfo
*/
void n_initcmds(nClass *cl)
{
    cl->BeginCmds();
    cl->EndCmds();
}

//-------------------------------------------------------------------
//  EOF
//-------------------------------------------------------------------
