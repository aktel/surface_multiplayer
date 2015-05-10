#ifndef SERVERCLASS_HEADER
#define SERVERCLASS_HEADER

#ifndef PRAGMA_ONCE
#pragma once
#endif

class ServerClass;
class SendTable;

extern ServerClass *g_pServerClassHead;


class ServerClass
{
public:
	ServerClass( char *pNetworkName, SendTable *pTable )
	{
		m_pNetworkName = pNetworkName;
		m_pTable = pTable;
		m_InstanceBaselineIndex = INVALID_STRING_INDEX;
		// g_pServerClassHead is sorted alphabetically, so find the correct place to insert
		if ( !g_pServerClassHead )
		{
			g_pServerClassHead = this;
			m_pNext = NULL;
		}
		else
		{
			ServerClass *p1 = g_pServerClassHead;
			ServerClass *p2 = p1->m_pNext;

			// use _stricmp because Q_stricmp isn't hooked up properly yet
			if ( _stricmp( p1->GetName(), pNetworkName ) > 0)
			{
				m_pNext = g_pServerClassHead;
				g_pServerClassHead = this;
				p1 = NULL;
			}

			while( p1 )
			{
				if ( p2 == NULL || _stricmp( p2->GetName(), pNetworkName ) > 0)
				{
					m_pNext = p2;
					p1->m_pNext = this;
					break;
				}
				p1 = p2;
				p2 = p2->m_pNext;
			}	
		}
	}

	const char*	GetName()		{ return m_pNetworkName; }


public:
	char						*m_pNetworkName;
	SendTable					*m_pTable;
	ServerClass					*m_pNext;
	int							m_ClassID;	// Managed by the engine.

	// This is an index into the network string table (sv.GetInstanceBaselineTable()).
	int							m_InstanceBaselineIndex; // INVALID_STRING_INDEX if not initialized yet.
};


#endif //