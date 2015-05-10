#ifndef _HEADER_SURFFUNC_H_
#define _HEADER_SURFFUNC_H_

#include <const.h>
#include <edict.h>

BEGIN_C
	
typedef struct
{
	void 	(*pfnRemoveEntity)( CEdict *pEnt );
	CEdict* (*pfnCreateNamedEntity)( string_t className );
	int 	(*pfnIndexOfEdict)( const CEdict* pEdict );
	
	void 	(*pfnClientCommand)( CEdict* pEdict, char *szFmt, ... );
	void 	(*pfnServerCommand)( const char *command );
	
	void 	(*pfnMessageBegin)( int msg_dest, int msg_num, CEdict* ed );
	void 	(*pfnMessageEnd)( void );
	
	void	(*pfnWriteByte)( int iValue );
	void	(*pfnWriteChar)( int iValue );
	void	(*pfnWriteShort)( int iValue );
	void 	(*pfnWriteLong)( int iValue );
	void 	(*pfnWriteCoort)( float flValue );
	void	(*pfnWriteEntity)( int iValue );
	
	void	(*pfnAlertMessage)( ALERT_TYPE level, char *szFmt, ... );
	void 	(*pfnEngineFprintf)( FILE *pFile, char *szFmt, .. );
	void 	(*pfnPvAllocEntPrivateData)( CEdict* pEdict, long cb );
	void	(*pfnPvEntPrivateData)( CEdict* pEdict );
	
	// Mem Allocator
	byte* 	(*pfnAllocPool)( const char *poolname, const char *filename, int line  );
	byte*	(*pfnAlloc)( byte* pool, uint size, const char *filename, int line );
	byte*	(*pfnRealloc)( byte *mem, const char *filename, int line );
	void 	(*pfnFree)( byte *mem );
	void 	(*pfnFreePool)( byte *mempool );
} surffunc_t;
	
END_C

#endif _HEADER_SURFFUNC_H_