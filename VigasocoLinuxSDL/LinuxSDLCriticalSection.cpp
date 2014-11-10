// LinuxSDLCriticalSection.cpp
//
/////////////////////////////////////////////////////////////////////////////

#include "LinuxSDLCriticalSection.h"

/////////////////////////////////////////////////////////////////////////////
// initialization and cleanup
/////////////////////////////////////////////////////////////////////////////

LinuxSDLCriticalSection::LinuxSDLCriticalSection()
{
}

LinuxSDLCriticalSection::~LinuxSDLCriticalSection()
{
}

/////////////////////////////////////////////////////////////////////////////
// ICriticalSection interface
/////////////////////////////////////////////////////////////////////////////

void LinuxSDLCriticalSection::init()
{
	cs=SDL_CreateMutex();
}

void LinuxSDLCriticalSection::destroy()
{
	SDL_DestroyMutex(cs);
}

void LinuxSDLCriticalSection::enter()
{
	SDL_mutexP(cs);
}

void LinuxSDLCriticalSection::leave()
{
	SDL_mutexV(cs);
}
