#ifndef LAUNCHER_H
#define LAUNCHER_H

#ifdef _WIN32
	#include <windows.h>
#else
	typedef void * HINSTANCE;
#endif

class ILauncher
{
public:	
	virtual void Launch( HINSTANCE hinstDLL ) = 0;
	virtual const char *GetGameStyle() = 0;
};

#endif