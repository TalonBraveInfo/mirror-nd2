//------------------------------------------------------------------------------
//  (C) 2004 Rafael Van Daele-Hunt
//------------------------------------------------------------------------------
#include "BombsquadBruce/ccworld.h"
#include "kernel/npersistserver.h"
#include "BombsquadBruce/general.h"

static void n_setmap(void* slf, nCmd* cmd);
static void n_startgameobjs(void* slf, nCmd* cmd);
static void n_addgameobject(void* slf, nCmd* cmd);
static void n_endgameobjs(void* slf, nCmd* cmd);
static void n_getheight(void* slf, nCmd* cmd);
static void n_isgameactive(void* slf, nCmd* cmd);
static void n_pause(void* slf, nCmd* cmd);

//------------------------------------------------------------------------------
/**
    @scriptclass
    ccworld

    @superclass
    nroot

    @classinfo
    The world is responsible for containing and handling the interaction 
    between the game objects (e.g. player & other stuff).

*/
void
n_initcmds(nClass* clazz)
{
    clazz->BeginCmds();
    clazz->AddCmd("v_setmap_s",             'STRN', n_setmap);
    clazz->AddCmd("v_startgameobjs_v",      'SADO', n_startgameobjs);
    clazz->AddCmd("b_addgameobject_s",      'ADOB', n_addgameobject);
    clazz->AddCmd("v_endgameobjs_v",        'EADO', n_endgameobjs);
    clazz->AddCmd("f_getheight_ff",         'GHGT', n_getheight );
    clazz->AddCmd("b_isgameactive_v",       'ISOV', n_isgameactive );
    clazz->AddCmd("v_pause_b",              'PAWS', n_pause );
    clazz->EndCmds();
}

//------------------------------------------------------------------------------
/**
@cmd
setmap

@input
s

@output
v

@info
Tells the world which map (nMap) to use
*/
static void n_setmap(void* slf, nCmd* cmd)
{
    CCWorld* self = (CCWorld*)slf;
    self->SetMap(cmd->In()->GetS());
}

//------------------------------------------------------------------------------
/**
@cmd
startgameobjs

@input
v

@output
v

@info
Begins the list of objects to be added (must be called before
the first call to addgameobject)
*/
static void n_startgameobjs(void* slf, nCmd* cmd)
{
    CCWorld* self = (CCWorld*)slf;
    self->StartGameObjs();
}
//------------------------------------------------------------------------------
/**
@cmd
addgameobject

@input
s

@output
b

@info
Adds an object to be rendered by the world
*/
static void n_addgameobject(void* slf, nCmd* cmd)
{
    CCWorld* self = (CCWorld*)slf;
    bool retVal = self->AddGameObject(cmd->In()->GetS());
    cmd->Out()->SetB( retVal );
}

//------------------------------------------------------------------------------
/**
@cmd
endgameobjs

@input
v

@output
v

@info
Ends the list of objects to be added (must be called after
the last call to addgameobject)
*/
static void n_endgameobjs(void* slf, nCmd* cmd)
{
    CCWorld* self = (CCWorld*)slf;
    self->EndGameObjs();
}
//------------------------------------------------------------------------------
/**
@cmd
getheight

@input 
ff	(x and z coordinates)

@output
f	(y coordinate) 

@info
Returns the height at the given position
*/
static void n_getheight(void* slf, nCmd* cmd)
{
    CCWorld* self = (CCWorld*)slf;
    float x = cmd->In()->GetF();
    float z = cmd->In()->GetF();
    cmd->Out()->SetF( self->GetHeight(x,z) );
}

//------------------------------------------------------------------------------
/**
@cmd
isgameactive

@input 
b

@output
v

@info
True iff the current game is running (not paused or over)
*/
static void n_isgameactive(void* slf, nCmd* cmd)
{
    CCWorld* self = (CCWorld*)slf;
    cmd->Out()->SetB( self->IsGameActive() );
}
//------------------------------------------------------------------------------
/**
@cmd
pause

@input 
b (true -> game will be paused, false -> unpaused)

@output
v

@info
Sets the game's pause state
*/
static void n_pause(void* slf, nCmd* cmd)
{
    CCWorld* self = (CCWorld*)slf;
    self->Pause(cmd->In()->GetB());
}
//------------------------------------------------------------------------------
/**
@param  fileServer  writes the nCmd object contents out to a file.
@return             success or failure
*/
bool
CCWorld::SaveCmds(nPersistServer* ps)
{
    if (nRoot::SaveCmds(ps))
    {
        // Step size
        CCUtil::SaveStrCmd doCmd( ps, this );
//        doCmd( m_rMap.getname(), 'STRN' );
        //doCmd( m_rPlayer.getname(), 'STPL' );
        //doCmd( this->m_GameWonFunc, 'SGTS' );
        //doCmd( this->m_GameWonFunc, 'SGWS' );
        //doCmd( this->m_GameLostFunc, 'SGLS' );

        //--- addgameobject ---
        // persistence isn't possible at the moment because we don't store the paths of added objects, and also don't distinguish between generic added objects and the player.
        //for( std::vector< CCRef<CCRoot> >::iterator i = m_GameObjs.begin(), m_GameObjs.end() != i; ++i )
        //{
        //    saveStrCmd m_GameObjs, doCmd( i->getname(), 'ADOB');
        //    ps->PutCmd(cmd);
        //}
        return true;
    }
    return false;
}