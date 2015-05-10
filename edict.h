#ifndef EDICT_HEADER
#define EDICT_HEADER

#ifdef PRAGMA_ONCE
#pragma once
#endif

#include <const.h>
#include <iunknown.h>

class CNetSerialNum
{
public:
	int m_NetworkSerialNumber;
	int m_NodeSerialNumber;
};

class CEdict
{
public:
	// Set when initting an entity. If it's only a networkable, this is false.
	void				SetEdict( IServerUnknown *pUnk, bool bFullEdict );

	const char *		GetClassName() const;
	
	bool				IsFree() const;
	void				SetFree();
	void				ClearFree();

	bool				HasStateChanged() const;
	void				ClearStateChanged();
	void				StateChanged();
	void				StateChanged( unsigned short offset );

	void				ClearTransmitState();
	
	void SetChangeInfo( unsigned short info );
	void SetChangeInfoSerialNumber( unsigned short sn );
	unsigned short	 GetChangeInfo() const;
	unsigned short	 GetChangeInfoSerialNumber() const;

public:

	bool	free;
	uint 	flags;
	
	
	CNetSerialNum serialnum;	// Game DLL sets this when it gets a serial number for its EHANDLE.

	INetworkable	*pnNetworkable;
	void *pvPrivateData;

protected:
	IUnknown		*pvUnk;		
};

#endif // EDICT_HEADER