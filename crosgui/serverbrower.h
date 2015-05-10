#ifndef CROSGUI_SERVERBROWER_H
#define CROSGUI_SERVERBROWER_H

#include <const.h>
//#include "crosgui.h"

#ifdef PRAGMA_ONCE
#pragma once
#endif

#include <datamap.h>

enum ServerViewSide
{
	SVS_INTERNET,
	SVS_LAN,
	SVS_FAVORITE,
}

class IServerBrower // : public IBaseUI
{
	virtual IServerBrower( void *pair, void *tabs );
	virtual void LockGameSelector( const char *game_name );
	virtual void PageView( ServerViewSide *side );

private:
	//friend class CSurfaceSetmaster;
	//friend class CFavoriteServers;
	friend class CServerScanner; 

	virtual void AddServer( CServerInfo *info );
	virtual void Clear();
	virtual boolean GameSelectorIsLock();
};

#endif