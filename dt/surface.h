#ifndef _ROOT_SURFACE_H

#include <platform.h>

#ifdef PRAGMA_ONCE
#pragma once
#endif

#include <common.h>

class ILauchSurface
{	
public:
	virtual int Version(); // GetVersion
	virtual int Begin(); // Begin
	virtual int Shutdown(); // Shutdown
	virtual int Pre();
	virtual int Post();

	#ifdef _WIN32
		virtual void RunFrom( HINSTANCE hInst );
		virtual qboolean LauncherInjected( HINSTANCE hInst );
	#endif
};

class ISurface
{
public:
};

#define _ROOT_SURFACE_H
#endif