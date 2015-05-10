#include <platform.h>
#include <const.h>
#include <system.h>

static HINSTANCE g_Instance;

void Launch()
{

}

void Shutdown()
{

}

#ifdef PROXY_DLL_LAUNCHER
boolean stdapi DllMain( HINSTANCE Instance, dword fdwReason, void* lpvReserved )
{
	g_Instance = Instance;	// Update Instance.

	switch ( fdwReason )
	{
		case DLL_PROCESS_ATTACH:
			Launch();
		
		case DLL_PROCESS_DEATTACH:
			Shutdown();
	}
}
#endif