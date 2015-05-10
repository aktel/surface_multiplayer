#ifndef VCO_HEADER
#define VCO_HEADER

#ifdef PRAGMA_ONCE
#pragma once
#endif

#include <surfgame.h>

template <typename EntType>
class CSurfPool
{
public:
	CSurfPool()
	{
		gmsurface.edicts = Mem_Alloc( mempool, sizeof( edict_t ) * surfeng->GetMaxEdicts() );
	}

	static CGMSurfPool gmsurface;
};

class CVCSurfGame : public CSurfGame
{
public:
	template < typename EntType >
	VGMSurfPool *GetSurfPool()
	{
		static CSurfPool<EntType> m_pool;
		return &m_pool.gmsurface;
	}
};

extern CVCSurfGame gmsurf;

#endif //VCO_HEADER