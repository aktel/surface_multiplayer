#ifndef GAMEDATA
#define GAMEDATA

#ifdef PRAGMA_ONCE
#pragma once
#endif

class CConstAddress
{
public:
	CConstAddress( const char *address_name, unsigned long address_dword ) 
	{ this->address_dword = address_dword; this->address_name = address_name; 
		static CConstAddress *m_ConstAddresHead = nullptr;
		
		pNext = nullptr;
	
		if ( !m_ConstAddresHead )
			m_ConstAddresHead = this;
		else
		{
			pNext = m_ConstAddresHead->pNext;
			m_ConstAddresHead->pNext = this;
		}
		
		for ( int i = 0; i<= sizeof(m_LastActivate); i++)
			m_LastActivate[i] = nullptr;
	}

	const char *address_name;
	unsigned long address_dword;
	
	void Debug()
	{
		for ( int i = 0; i<= sizeof( m_LastActivate )-1;  )
		{
			m_LastActivate[i] = m_LastActivate[++i];
			m_LastActivate[0] = this;
		}
	}
	
	static CConstAddress *m_LastActivate[100];
	
	CConstAddress* pNext;
}

#endif // GAMEDATA