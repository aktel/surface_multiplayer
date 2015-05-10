#include <surface.h>
#include <platform.h>
#include <launcher.h>

class CLauncher : public ILauncher
{
public:
	virtual void Launch( HINSTANCE hinstDLL )
	{
		ILauchSurface* (*pfnGetSurfInterface)();

		HMODULE hMod = GetModuleHandleA( "surface.dll" );
			
		if ( !hMod )
			ExitProcess(0);
			
		
		pfnGetSurfInterface = (void(*)(void))GetProcAddress( hMod, "GetSurfInterface" );
		
		if ( !pfnGetSurfInterface )
			ExitProcess(0);
		
		ILauchSurface* surf = pfnGetSurfInterface();
			
		if ( !surf )
			ExitProcess(0);
			
		// if a readly launch when skip RunFrom
		if ( surf->LauncherInjected( hinstDLL ) )
			surf->RunFrom( hinstDLL ); 
	}

	virtual const char *GetGameStyle()
	{
		static const char style [] = "";
		return &style[0];
	}

	ILauncher *GetLauncherInterface()
	{
		return this;
	}
};

CLauncher launcher;

EXPORT ILauncher * GetLauncherInterface()
{
	return launcher.GetLauncherInterface();
} 

BOOL WINAPI DllMain(
  _In_ HINSTANCE hinstDLL,
  _In_ DWORD     fdwReason,
  _In_ LPVOID    lpvReserved
)
{
	ISurface *(*pfnGetSurfInterface)();
	
	switch( fdwReason )
	{
		case DLL_PROCESS_ATTACH:
		{
			launcher.Launch( hinstDLL );
			break;
		}
	}
	
	return TRUE;
}