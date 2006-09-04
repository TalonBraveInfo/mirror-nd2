//------------------------------------------------------------------------------
//  fsm/state.cc
//  (C) 2005 RadonLabs GmbH
//------------------------------------------------------------------------------
#include "fsm/state.h"

namespace FSM
{
ImplementRtti(FSM::State, Foundation::RefCounted);

//------------------------------------------------------------------------------
/**
*/
State::State(const nString& name) :
    name(name)
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
State::~State()
{
    // empty
}
//------------------------------------------------------------------------------
/**
*/
void
State::Trigger()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
State::OnEnter()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
void
State::OnLeave()
{
    // empty
}

//------------------------------------------------------------------------------
/**
*/
const nString&
State::GetName() const
{
    return this->name;
}

} // namespace FSM
