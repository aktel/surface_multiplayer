#ifndef _HEADER_SERVERINTERFACE_H_
#define _HEADER_SERVERINTERFACE_H_

#ifdef PRAGMA_ONCE
#pragma once
#endif

#include <const.h>
#include <edict.h>

BEGIN_C

typedef struct
{
	void 	(*pfnClientPutInServer)( CEdict *pEnt );
	void 	(*pfnClientConnect)( CEdict *pEnt, const char *pszName, const char *szRejectReason );
	void 	(*pfnClientDisconnect)( CEdict *pClient );
	void 	(*pfnOnFreeEntPrivateData)( CEdict *pEnt );
	
	void 	(*pfnCreateInstancedBaselines)();
	void	(*pfnShouldTransmit)( CEdict *client, CEdict *entity, qboolean isPlayer );
	qboolean	(*pfnEntStateFill)( CEdict *edict, CEntState *state );
	void 	(*pfnServerActivate)( CEdict *edicts, uint numEnt, uint maxClients );
	void 	(*pfnServerDeactivate)();
	
	void 	(*pfnCreateEntity)( CEdict *pEnt, const char *szClassName );
	
	void 	(*pfnAllocString)( const char *szValue );
	void 	(*pfnMakeString)( const char *szValue );
	void 	(*pfnGetString)( string_t iString );
	
	void 	(*pfnSys_Error)( const char *err_string );
} gamefuncs_t;

END_C

#endif // _HEADER_SERVERINTERFACE_H_