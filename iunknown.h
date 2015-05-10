#ifndef IUNKNOWN_HEADER
#define IUNKNOWN_HEADER

#ifdef PRAGMA_ONCE
#pragma once
#endif

class edict_t;

class IUnknown
{
public:
	virtual void CreateEnt( edict_t *edict );
	virtual void DispatchValue( edict_t *edict, const char *key, const char *value );
	virtual void Connect( edict_t *edict );
	virtual void Disconnect( edict_t *edict );
	virtual void Reconnect( edict_t *edict );
	virtual void Activate( edict_t *edict );
	virtual void Distroy( edict_t *edict );
};

typedef IUnknown IServerUnknown;

#endif // IUNKNOWN_HEADER