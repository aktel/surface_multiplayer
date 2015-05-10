#include <common.h>
#include "server.h"

void SV_SysError( const char *error_string )
{
	if( svgame.hInstance != NULL )
		svgame.func.pfnSys_Error( error_string );
}

/*
=================
SV_Send

Sends the contents of sv.multicast to a subset of the clients,
then clears sv.multicast.

MSG_ONE	send to one client (ent can't be NULL)
MSG_ALL	same as broadcast (origin can be NULL)
MSG_PVS	send to clients potentially visible from org
MSG_PHS	send to clients potentially hearable from org
=================
*/
qboolean SV_Send( int dest, const vec3_t origin, const edict_t *ent )
{
	byte		*mask = NULL;
	int		j, numclients = sv_maxclients->integer;
	CSVClient	*cl, *current = svs.clients;
	qboolean		reliable = false;
	qboolean		specproxy = false;
	int		numsends = 0;
	mleaf_t		*leaf;

	switch( dest )
	{
	case MSG_INIT:
		if( sv.state == ss_loading )
		{
			// copy to signon buffer
			BF_WriteBits( &sv.signon, BF_GetData( &sv.multicast ), BF_GetNumBitsWritten( &sv.multicast ));
			BF_Clear( &sv.multicast );
			return true;
		}
		// intentional fallthrough (in-game MSG_INIT it's a MSG_ALL reliable)
	case MSG_ALL:
		reliable = true;
		// intentional fallthrough
	case MSG_BROADCAST:
		// nothing to sort	
		break;
	case MSG_ONE:
		reliable = true;
		// intentional fallthrough
	case MSG_ONE_UNRELIABLE:
		if( ent == NULL ) return false;
		j = NUM_FOR_EDICT( ent );
		if( j < 1 || j > numclients ) return false;
		current = svs.clients + (j - 1);
		numclients = 1; // send to one
		break;
	//case MSG_SPEC:
	//	specproxy = reliable = true;
	//	break;
	default:
		Host_Error( "SV_Send: bad dest: %i\n", dest );
		return false;
	}

	// send the data to all relevent clients (or once only)
	for( j = 0, cl = current; j < numclients; j++, cl++ )
	{
		if( cl->state == cs_free || cl->state == cs_zombie )
			continue;

		if( cl->state != cs_spawned && !reliable )
			continue;

		if( !cl->edict || cl->fakeclient )
			continue;

		if( ent != NULL && ent->v.groupinfo && cl->edict->v.groupinfo )
		{
			if(( !svs.groupop && !( cl->edict->v.groupinfo & ent->v.groupinfo )) || (svs.groupop == 1 && ( cl->edict->v.groupinfo & ent->v.groupinfo ) != 0 ))
				continue;
		}

		if( !SV_CheckClientVisiblity( cl, mask ))
			continue;

		if( specproxy ) BF_WriteBits( &sv.spectator_datagram, BF_GetData( &sv.multicast ), BF_GetNumBitsWritten( &sv.multicast ));
		else if( reliable ) BF_WriteBits( &cl->netchan.message, BF_GetData( &sv.multicast ), BF_GetNumBitsWritten( &sv.multicast ));
		else BF_WriteBits( &cl->datagram, BF_GetData( &sv.multicast ), BF_GetNumBitsWritten( &sv.multicast ));
		numsends++;
	}

	BF_Clear( &sv.multicast );

	return numsends;	// debug
}

/*
=======================
SV_GetReliableDatagram

Get shared reliable buffer
=======================
*/
sizebuf_t *SV_GetReliableDatagram( void )
{
	return &sv.reliable_datagram;
}

void SV_CreateStaticEntity( sizebuf_t *bf, ent_class_t *ent_class )
{

}

void SV_FreePrivateData( CEdict *pEdict )
{
	if( !pEdict || !pEdict->pvPrivateData )
		return;
	
	svgame.func.pfnOnFreeEntPrivateData( pEdict );

	if( Mem_IsAllocatedExt( svgame.mempool, pEdict->pvPrivateData ))
		Mem_Free( pEdict->pvPrivateData );

	pEdict->pvPrivateData = NULL;
}

void SV_FreeEdict( CEdict *pEdict )
{
	ASSERT( pEdict );
	ASSERT( pEdict->free == false );

	SV_FreePrivateData( pEdict );

	// NOTE: don't clear all edict fields on releasing
	// because gamedll may trying to use edict pointers and crash game (e.g. Opposing Force)

	// mark edict as freed
	pEdict->freetime = sv.time;
	pEdict->serialnumber++; // invalidate EHANDLE's
	pEdict->free = true;
}

CEdict *SV_AllocEdict( void )
{
	CEdict *pEdict;
	int i;

	for ( i = svgame.globals->maxPlayers + 1; i < svgame.numEntities; i++ )
	{
		pEdict = EDICT_NUM( i );
	}

	if ( pEdict->free && ( pEdict->freetime < 2.0f || ( sv.time - pEdict->freetime ) > 0.5f  ) )
	{
		SV_InitEdict( pEdict );
		return pEdict;
	}

	if ( i >= svgame.globals->maxEntities )
	{
		Sys_Error( "SV_AllocEdict: no free edicts\n" );
	}

	svgame.numEntities++;
	pEdict->EDICT_NUM( pEdict );
	SV_InitEdict( pEdict );
	
	return pEdict;
}

CEdict *SV_AllocPrivateData( edict_t *ent, string_t className )
{
	const char	*pszClassName;
	LINK_ENTITY_FUNC	SpawnEdict;

	pszClassName = CLASSNAME_STRING( className );

	if( !ent )
	{
		// allocate a new one
		ent = SV_AllocEdict();
	}
	else if( ent->free )
	{
		SV_InitEdict( ent ); // re-init edict
		MsgDev( D_WARN, "SV_AllocPrivateData: entity %s is freed!\n", CLASSNAME_STRING( className ));
	}

	ent->v.classname = className;
	ent->v.pContainingEntity = ent; // re-link
	
	// allocate edict private memory (passed by dlls)
	SpawnEdict = (LINK_ENTITY_FUNC)Com_GetProcAddress( svgame.hInstance, pszClassName );

	if( !SpawnEdict )
	{
		SpawnEdict = (LINK_ENTITY_FUNC)svgame.spawnfuncs[className][0];

		if ( SpawnEdict )
			goto spawn;

		if( svgame.func.pfnCreateEntity && svgame.physFuncs.pfnCreateEntity( ent, pszClassName ) != -1 )
			return ent;

		MsgDev( D_ERROR, "No spawn function for %s\n", CLASSNAME_STRING( className ));

		// kill entity immediately
		SV_FreeEdict( ent );

		return NULL;
	}
	
spawn:
	SpawnEdict( ent );

	return ent;
}

void SV_FreeEdicts( void )
{
	int	i = 0;
	edict_t	*ent;

	for( i = 0; i < svgame.numEntities; i++ )
	{
		ent = EDICT_NUM( i );
		if( ent->free ) continue;
		SV_FreeEdict( ent );
	}
}

CSVClient *SV_ClientFromEdict( const edict_t *pEdict, qboolean spawned_only )
{
	int	i;

	if( !SV_IsValidEdict( pEdict ))
		return NULL;

	i = NUM_FOR_EDICT( pEdict ) - 1;

	if( i < 0 || i >= sv_maxclients->integer )
		return NULL;

	if( spawned_only )
	{
		if( svs.clients[i].state != cs_spawned )
			return NULL;
	}

	return (svs.clients + i);

}


/*
=========
SV_BaselineForEntity

assume pEdict is valid
=========
*/
void SV_BaselineForEntity( edict_t *pEdict )
{
	int		usehull, player;
	int		modelindex;
	entity_state_t	baseline;
	float		*mins, *maxs;
	CSVClient	*cl;

	if( pEdict->v.flags & FL_CLIENT && ( cl = SV_ClientFromEdict( pEdict, false )))
	{
		usehull = ( pEdict->v.flags & FL_DUCKING ) ? true : false;
		modelindex = cl->modelindex ? cl->modelindex : pEdict->v.modelindex;
		mins = svgame.player_mins[usehull]; 
		maxs = svgame.player_maxs[usehull]; 
		player = true;
	}
	else
	{
		if( pEdict->v.effects == EF_NODRAW )
			return;

		if( !pEdict->v.modelindex || !STRING( pEdict->v.model ))
			return; // invisible

		modelindex = pEdict->v.modelindex;
		mins = pEdict->v.mins; 
		maxs = pEdict->v.maxs; 
		player = false;
	}

	// take current state as baseline
	Q_memset( &baseline, 0, sizeof( baseline )); 
	baseline.number = NUM_FOR_EDICT( pEdict );

	svgame.func.pfnCreateBaseline( player, baseline.number, &baseline, pEdict, modelindex, mins, maxs );

	// set entity type
	if( pEdict->v.flags & FL_CUSTOMENTITY )
		baseline.entityType = ENTITY_BEAM;
	else baseline.entityType = ENTITY_NORMAL;

	svs.baselines[baseline.number] = baseline;
}

/*
==============
pfnRemoveEntity

free edict private mem, unlink physics etc
==============
*/
void pfnRemoveEntity( CEdict* e )
{
	if( !SV_IsValidEdict( e ))
	{
		MsgDev( D_ERROR, "SV_RemoveEntity: entity already freed\n" );
		return;
	}

	// never free client or world entity
	if( NUM_FOR_EDICT( e ) < ( svgame.globals->maxClients + 1 ))
	{
		MsgDev( D_ERROR, "SV_RemoveEntity: can't delete %s\n", (e == EDICT_NUM( 0 )) ? "world" : "client" );
		return;
	}

	SV_FreeEdict( e );
}

/*
==============
pfnCreateNamedEntity

==============
*/
CEdict* pfnCreateNamedEntity( string_t className )
{
	return SV_AllocPrivateData( NULL, className );
}

/*
=========
pfnServerCommand

=========
*/
void pfnServerCommand( const char* str )
{
	Cbuf_AddText( str );
}

/*
=========
pfnClientCommand

=========
*/
void pfnClientCommand( CEdict* pEdict, char* szFmt, ... )
{
	CSVClient	*client;
	string		buffer;
	va_list		args;

	if( sv.state != ss_active )
	{
		MsgDev( D_ERROR, "SV_ClientCommand: server is not active!\n" );
		return;
	}

	if(( client = SV_ClientFromEdict( pEdict, true )) == NULL )
	{
		MsgDev( D_ERROR, "SV_ClientCommand: client is not spawned!\n" );
		return;
	}

	if( client->fakeclient )
		return;

	va_start( args, szFmt );
	Q_vsnprintf( buffer, MAX_STRING, szFmt, args );
	va_end( args );

	//if( SV_IsValidCmd( buffer ))
	{
		BF_WriteSvc( client->curr_chan, svc_stufftext );
		BF_WriteString( client->curr_chan, buffer );
	}
	//else MsgDev( D_ERROR, "Tried to stuff bad command %s\n", buffer );
}

/*
=============
pfnMessageBegin

=============
*/
void pfnMessageBegin( int msg_dest, int msg_num, CEdict *ed )
{
	int	i, iSize;

	if( svgame.msg_started )
		Host_Error( "MessageBegin: New message started when msg '%s' has not been sent yet\n", svgame.msg_name );
	svgame.msg_started = true;

	// check range
	msg_num = bound( net_bad, msg_num, 255 );

	if( msg_num < svc_lastmsg )
	{
		svgame.msg_index = -msg_num; // this is a system message
		svgame.msg_name = svc_strings[msg_num];

		iSize = 0;
	}
	else
	{
		// check for existing
		for( i = 0; i < MAX_USER_MESSAGES && svgame.msg[i].name[0]; i++ )
		{
			if( svgame.msg[i].number == msg_num )
				break; // found
		}

		if( i == MAX_USER_MESSAGES )
		{
			Host_Error( "MessageBegin: tried to send unregistered message %i\n", msg_num );
			return;
		}

		svgame.msg_name = svgame.msg[i].name;
		iSize = svgame.msg[i].size;
		svgame.msg_index = i;
	}

	BF_WriteByte( &sv.multicast, msg_num );

	// save message destination
	if( pOrigin ) VectorCopy( pOrigin, svgame.msg_org );
	else VectorClear( svgame.msg_org );

	if( iSize == -1 )
	{
		// variable sized messages sent size as first byte
		svgame.msg_size_index = BF_GetNumBytesWritten( &sv.multicast );
		BF_WriteByte( &sv.multicast, 0 ); // reserve space for now
	}
	else svgame.msg_size_index = -1; // message has constant size

	svgame.msg_realsize = 0;
	svgame.msg_dest = msg_dest;
	svgame.msg_ent = ed;
}

/*
=============
pfnMessageEnd

=============
*/
void pfnMessageEnd( void )
{
	const char	*name = "Unknown";
	float		*org = NULL;

	if( svgame.msg_name ) name = svgame.msg_name;
	if( !svgame.msg_started ) Host_Error( "MessageEnd: called with no active message\n" );
	svgame.msg_started = false;

	// HACKHACK: clearing HudText in background mode
	if( sv.background && svgame.msg_index >= 0 && svgame.msg[svgame.msg_index].number == svgame.gmsgHudText )
	{
		BF_Clear( &sv.multicast );
		return;
	}

	// check for system message
	if( svgame.msg_index < 0 )
	{
		if( svgame.msg_size_index != -1 )
		{
			// variable sized message
			if( svgame.msg_realsize > 255 )
			{
				MsgDev( D_ERROR, "SV_Message: %s too long (more than 255 bytes)\n", name );
				BF_Clear( &sv.multicast );
				return;
			}
			else if( svgame.msg_realsize < 0 )
			{
				MsgDev( D_ERROR, "SV_Message: %s writes NULL message\n", name );
				BF_Clear( &sv.multicast );
				return;
			}
		}

		sv.multicast.pData[svgame.msg_size_index] = svgame.msg_realsize;
	}
	else if( svgame.msg[svgame.msg_index].size != -1 )
	{
		int	expsize = svgame.msg[svgame.msg_index].size;
		int	realsize = svgame.msg_realsize;
	
		// compare sizes
		if( expsize != realsize )
		{
			MsgDev( D_ERROR, "SV_Message: %s expected %i bytes, it written %i. Ignored.\n", name, expsize, realsize );
			BF_Clear( &sv.multicast );
			return;
		}
	}
	else if( svgame.msg_size_index != -1 )
	{
		// variable sized message
		if( svgame.msg_realsize > 255 )
		{
			MsgDev( D_ERROR, "SV_Message: %s too long (more than 255 bytes)\n", name );
			BF_Clear( &sv.multicast );
			return;
		}
		else if( svgame.msg_realsize < 0 )
		{
			MsgDev( D_ERROR, "SV_Message: %s writes NULL message\n", name );
			BF_Clear( &sv.multicast );
			return;
		}

		sv.multicast.pData[svgame.msg_size_index] = svgame.msg_realsize;
	}
	else
	{
		// this should never happen
		MsgDev( D_ERROR, "SV_Message: %s have encountered error\n", name );
		BF_Clear( &sv.multicast );
		return;
	}

	if( svgame.msg_index < 0 && abs( svgame.msg_index ) == svc_studiodecal && svgame.msg_realsize == 27 )
	{
		// oldstyle message for svc_studiodecal has missed four additional bytes:
		// modelIndex, skin and body. Write it here for backward compatibility
		BF_WriteWord( &sv.multicast, 0 );
		BF_WriteByte( &sv.multicast, 0 );
		BF_WriteByte( &sv.multicast, 0 );
	}

	if( !VectorIsNull( svgame.msg_org )) org = svgame.msg_org;
	svgame.msg_dest = bound( MSG_BROADCAST, svgame.msg_dest, MSG_SPEC );

	SV_Send( svgame.msg_dest, org, svgame.msg_ent );
}

/*
=============
pfnWriteByte

=============
*/
void pfnWriteByte( int iValue )
{
	if( iValue == -1 ) iValue = 0xFF; // convert char to byte 
	BF_WriteByte( &sv.multicast, (byte)iValue );
	svgame.msg_realsize++;
}

/*
=============
pfnWriteChar

=============
*/
void pfnWriteChar( int iValue )
{
	BF_WriteChar( &sv.multicast, (char)iValue );
	svgame.msg_realsize++;
}

/*
=============
pfnWriteShort

=============
*/
void pfnWriteShort( int iValue )
{
	BF_WriteShort( &sv.multicast, (short)iValue );
	svgame.msg_realsize += 2;
}

/*
=============
pfnWriteLong

=============
*/
void pfnWriteLong( int iValue )
{
	BF_WriteLong( &sv.multicast, iValue );
	svgame.msg_realsize += 4;
}

/*
=============
pfnWriteAngle

this is low-res angle
=============
*/
void pfnWriteAngle( float flValue )
{
	int	iAngle = ((int)(( flValue ) * 256 / 360) & 255);

	BF_WriteChar( &sv.multicast, iAngle );
	svgame.msg_realsize += 1;
}

/*
=============
pfnWriteCoord

=============
*/
void pfnWriteCoord( float flValue )
{
	BF_WriteCoord( &sv.multicast, flValue );
	svgame.msg_realsize += 2;
}

/*
=============
pfnWriteEntity

=============
*/
void pfnWriteEntity( int iValue )
{
	if( iValue < 0 || iValue >= svgame.numEntities )
		Host_Error( "BF_WriteEntity: invalid entnumber %i\n", iValue );
	BF_WriteShort( &sv.multicast, (short)iValue );
	svgame.msg_realsize += 2;
}

/*
=============
pfnAlertMessage

=============
*/
static void pfnAlertMessage( ALERT_TYPE level, char *szFmt, ... )
{
	char	buffer[2048];	// must support > 1k messages
	va_list	args;

	// check message for pass
	switch( level )
	{
	case at_notice:
		break;	// passed always
	case at_console:
		if( host.developer < D_INFO )
			return;
		break;
	case at_aiconsole:
		if( host.developer < D_AICONSOLE )
			return;
		break;
	case at_warning:
		if( host.developer < D_WARN )
			return;
		break;
	case at_error:
		if( host.developer < D_ERROR )
			return;
		break;
	}

	va_start( args, szFmt );
	Q_vsnprintf( buffer, 2048, szFmt, args );
	va_end( args );

	if( level == at_warning )
	{
		Sys_Print( va( "^3Warning:^7 %s", buffer ));
	}
	else if( level == at_error  )
	{
		Sys_Print( va( "^1Error:^7 %s", buffer ));
	} 
	else
	{
		Sys_Print( buffer );
	}
}

/*
=============
pfnEngineFprintf

legacy. probably was a part of early save\restore system
=============
*/
static void pfnEngineFprintf( FILE *pfile, char *szFmt, ... )
{
	char	buffer[2048];
	va_list	args;

	va_start( args, szFmt );
	Q_vsnprintf( buffer, 2048, szFmt, args );
	va_end( args );

	fprintf( pfile, buffer );
}

/*
=============
pfnPvAllocEntPrivateData

=============
*/
void *pfnPvAllocEntPrivateData( CEdict *pEdict, long cb )
{
	ASSERT( pEdict );

	SV_FreePrivateData( pEdict );

	if( cb > 0 )
	{
		// a poke646 have memory corrupt in somewhere - this is trashed last four bytes :(
		pEdict->pvPrivateData = Mem_Alloc( svgame.mempool, (cb + 15) & ~15 );
	}

	return pEdict->pvPrivateData;
}

/*
=============
pfnPvEntPrivateData

we already have copy of this function in 'enginecallback.h' :-)
=============
*/
void *pfnPvEntPrivateData( CEdict *pEdict )
{
	if( pEdict )
		return pEdict->pvPrivateData;
	return NULL;
}

/*
=============
pfnFreeEntPrivateData

=============
*/
void pfnFreeEntPrivateData( edict_t *pEdict )
{
	SV_FreePrivateData( pEdict );
}

/*
=============
SV_AllocString

allocate new engine string
=============
*/
string_t SV_AllocString( const char *szValue )
{
	const char *newString;

	if( svgame.func.pfnAllocString != NULL )
		return svgame.func.pfnAllocString( szValue );

	newString = _copystring( svgame.stringspool, szValue, __FILE__, __LINE__ );
	return newString - svgame.globals->pStringBase;
}		

/*
=============
SV_MakeString

make constant string
=============
*/
string_t SV_MakeString( const char *szValue )
{
	if( svgame.func.pfnMakeString != NULL )
		return svgame.func.pfnMakeString( szValue );
	return szValue - svgame.globals->pStringBase;
}		


/*
=============
SV_GetString

=============
*/
const char *SV_GetString( string_t iString )
{
	if( svgame.func.pfnGetString != NULL )
		return svgame.func.pfnGetString( iString );
	return (svgame.globals->pStringBase + iString);
}

/*
=============
pfnPEntityOfEntOffset

=============
*/
edict_t* pfnPEntityOfEntOffset( int iEntOffset )
{
	return (&((edict_t*)svgame.vp)[iEntOffset]);
}

/*
=============
pfnEntOffsetOfPEntity

=============
*/
int pfnEntOffsetOfPEntity( const edict_t *pEdict )
{
	return ((byte *)pEdict - (byte *)svgame.vp);
}

/*
=============
pfnIndexOfEdict

=============
*/
int pfnIndexOfEdict( const CEdict *pEdict )
{
	int	number;

	number = NUM_FOR_EDICT( pEdict );
	if( number < 0 || number >= svgame.numEntities )
		return 0;	// out of range
	return number;
}

/*
=============
pfnPEntityOfEntIndex

=============
*/
edict_t* pfnPEntityOfEntIndex( int iEntIndex )
{
	if( iEntIndex < 0 || iEntIndex >= svgame.numEntities )
		return NULL; // out of range

	return EDICT_NUM( iEntIndex );
}

/*
=============
pfnRegUserMsg

=============
*/
int pfnRegUserMsg( const char *pszName, int iSize )
{
	int	i;
	
	if( !pszName || !pszName[0] )
		return net_bad;

	if( Q_strlen( pszName ) >= sizeof( svgame.msg[0].name ))
	{
		MsgDev( D_ERROR, "REG_USER_MSG: too long name %s\n", pszName );
		return net_bad; // force error
	}

	if( iSize > 255 )
	{
		MsgDev( D_ERROR, "REG_USER_MSG: %s has too big size %i\n", pszName, iSize );
		return net_bad; // force error
	}

	// make sure what size inrange
	iSize = bound( -1, iSize, 255 );

	// message 0 is reserved for svc_bad
	for( i = 0; i < MAX_USER_MESSAGES && svgame.msg[i].name[0]; i++ )
	{
		// see if already registered
		if( !Q_strcmp( svgame.msg[i].name, pszName ))
			return svc_lastmsg + i; // offset
	}

	if( i == MAX_USER_MESSAGES ) 
	{
		MsgDev( D_ERROR, "REG_USER_MSG: user messages limit exceeded\n" );
		return net_bad;
	}

	// register new message
	Q_strncpy( svgame.msg[i].name, pszName, sizeof( svgame.msg[i].name ));
	svgame.msg[i].number = svc_lastmsg + i;
	svgame.msg[i].size = iSize;

	// catch some user messages
	if( !Q_strcmp( pszName, "HudText" ))
		svgame.gmsgHudText = svc_lastmsg + i;

	if( sv.state == ss_active )
	{
		// tell the client about new user message
		BF_WriteSvc( &sv.multicast, svc_usermessage );
		BF_WriteByte( &sv.multicast, svgame.msg[i].number );
		BF_WriteByte( &sv.multicast, (byte)iSize );
		BF_WriteString( &sv.multicast, svgame.msg[i].name );
		SV_Send( MSG_ALL, NULL, NULL );
	}

	return svgame.msg[i].number;
}

/*
=============
pfnClientPrintf

=============
*/
void pfnClientPrintf( edict_t* pEdict, PRINT_TYPE ptype, const char *szMsg )
{
	sv_client_t	*client;

	if( sv.state != ss_active )
	{
		// send message into console during loading
		MsgDev( D_INFO, szMsg );
		return;
	}

	if(( client = SV_ClientFromEdict( pEdict, true )) == NULL )
	{
		MsgDev( D_ERROR, "SV_ClientPrintf: client is not spawned!\n" );
		return;
	}

	switch( ptype )
	{
	case print_console:
		if( client->fakeclient ) MsgDev( D_INFO, szMsg );
		else SV_ClientPrintf( client, PRINT_HIGH, "%s", szMsg );
		break;
	case print_chat:
		if( client->fakeclient ) return;
		SV_ClientPrintf( client, PRINT_CHAT, "%s", szMsg );
		break;
	}
}

/*
=============
pfnServerPrint

=============
*/
void pfnServerPrint( const char *szMsg )
{
	// while loading in-progress we can sending message only for local client
	if( sv.state != ss_active ) MsgDev( D_INFO, szMsg );	
	else SV_BroadcastPrintf( PRINT_HIGH, "%s", szMsg );
}

/*
=============
pfnCheckParm

=============
*/
static int pfnCheckParm( char *parm, char **ppnext )
{
	static char	str[64];

	if( Sys_GetParmFromCmdLine( parm, str ))
	{
		// get the pointer on cmdline param
		if( ppnext ) *ppnext = str;
		return 1;
	}
	return 0;
}

/*
====================
SV_ParseEdict

Parses an edict out of the given string, returning the new position
ed should be a properly initialized empty edict.
====================
*/
qboolean SV_ParseEdict( char **pfile, edict_t *ent )
{
	KeyValueData	pkvd[256]; // per one entity
	int		i, numpairs = 0;
	const char	*classname = NULL;
	char		token[2048];

#if 0
	// go through all the dictionary pairs
	while( 1 )
	{	
		string	keyname;

		// parse key
		if(( *pfile = COM_ParseFile( *pfile, token )) == NULL )
			Host_Error( "SV_ParseEdict: EOF without closing brace\n" );
		if( token[0] == '}' ) break; // end of desc

		Q_strncpy( keyname, token, sizeof( keyname ));

		// parse value	
		if(( *pfile = COM_ParseFile( *pfile, token )) == NULL ) 
			Host_Error( "SV_ParseEdict: EOF without closing brace\n" );

		if( token[0] == '}' )
			Host_Error( "SV_ParseEdict: closing brace without data\n" );

		// ignore attempts to set key ""
		if( !keyname[0] ) continue;

		// "wad" field is completely ignored in Xash3D
		if( !Q_strcmp( keyname, "wad" ))
			continue;

		// keynames with a leading underscore are used for utility comments,
		// and are immediately discarded by engine
		if( world.version == Q1BSP_VERSION && keyname[0] == '_' )
			continue;

		// ignore attempts to set value ""
		if( !token[0] ) continue;

		// create keyvalue strings
		pkvd[numpairs].szClassName = (char *)classname;	// unknown at this moment
		pkvd[numpairs].szKeyName = copystring( keyname );
		pkvd[numpairs].szValue = copystring( token );
		pkvd[numpairs].fHandled = false;		

		if( !Q_strcmp( keyname, "classname" ) && classname == NULL )
			classname = pkvd[numpairs].szValue;
		if( ++numpairs >= 256 ) break;
	}
	
	ent = SV_AllocPrivateData( ent, ALLOC_STRING( classname ));

	if( !SV_IsValidEdict( ent ) || ent->v.flags & FL_KILLME )
	{
		// release allocated strings
		for( i = 0; i < numpairs; i++ )
		{
			Mem_Free( pkvd[i].szKeyName );
			Mem_Free( pkvd[i].szValue );
		}
		return false;
	}

	for( i = 0; i < numpairs; i++ )
	{
		if( !Q_strcmp( pkvd[i].szKeyName, "angle" ))
		{
			float	flYawAngle = Q_atof( pkvd[i].szValue );

			Mem_Free( pkvd[i].szKeyName ); // will be replace with 'angles'
			Mem_Free( pkvd[i].szValue );	// release old value, so we don't need these
			pkvd[i].szKeyName = copystring( "angles" );

			if( flYawAngle >= 0.0f )
				pkvd[i].szValue = copystring( va( "%g %g %g", ent->v.angles[0], flYawAngle, ent->v.angles[2] ));
			else if( flYawAngle == -1.0f )
				pkvd[i].szValue = copystring( "-90 0 0" );
			else if( flYawAngle == -2.0f )
				pkvd[i].szValue = copystring( "90 0 0" );
			else pkvd[i].szValue = copystring( "0 0 0" ); // technically an error
		}

		if( !Q_strcmp( pkvd[i].szKeyName, "light" ))
		{
			Mem_Free( pkvd[i].szKeyName );
			pkvd[i].szKeyName = copystring( "light_level" );
		}

		if( !pkvd[i].fHandled )
		{
			pkvd[i].szClassName = (char *)classname;
			svgame.dllFuncs.pfnKeyValue( ent, &pkvd[i] );
		}

		// no reason to keep this data
		Mem_Free( pkvd[i].szKeyName );
		Mem_Free( pkvd[i].szValue );
	}
#endif

	return true;
}

/*
================
SV_LoadFromFile

The entities are directly placed in the array, rather than allocated with
ED_Alloc, because otherwise an error loading the map would have entity
number references out of order.

Creates a server's entity / program execution context by
parsing textual entity definitions out of an ent file.
================
*/
void SV_LoadFromFile( const char *mapname, char *entities )
{
	char	token[2048];
	qboolean	create_world = true;
	int	inhibited;
	edict_t	*ent;

	ASSERT( entities != NULL );

#if 0
	// user dll can override spawn entities function (Xash3D extension)
	if( !svgame.physFuncs.SV_LoadEntities || !svgame.physFuncs.SV_LoadEntities( mapname, entities ))
	{
		inhibited = 0;

		// parse ents
		while(( entities = COM_ParseFile( entities, token )) != NULL )
		{
			if( token[0] != '{' )
				Host_Error( "SV_LoadFromFile: found %s when expecting {\n", token );

			if( create_world )
			{
				create_world = false;
				ent = EDICT_NUM( 0 ); // already initialized
			}
			else ent = SV_AllocEdict();

			if( !SV_ParseEdict( &entities, ent ))
				continue;

			if( svgame.dllFuncs.pfnSpawn( ent ) == -1 )
			{
				// game rejected the spawn
				if( !( ent->v.flags & FL_KILLME ))
				{
					SV_FreeEdict( ent );
					inhibited++;
				}
			}
		}

		MsgDev( D_INFO, "\n%i entities inhibited\n", inhibited );
	}

#endif
}


qboolean SV_LoadProgs( const char *name )
{
	int			i, version;

	#ifdef SURFACE_STATIC_GAME

		static APIFUNCTION		GetEntityAPI;
		static APIFUNCTION2		GetEntityAPI2;
		static GIVEFNPTRSTODLL	GiveFnptrsToDll;
		static surffuncs_t	gpSurffuncs;
		static globalvars_t		gpGlobals;
		static playermove_t		gpMove;

	#endif

	#ifdef SURFACE_STATIC_GAME
		if( svgame.hInstance ) SV_UnloadProgs();
	#endif

	// fill it in
	svgame.globals = &gpGlobals;
	svgame.mempool = Mem_AllocPool( "Server Edicts Zone" );
	#ifdef SURFACE_STATIC_GAME
		svgame.hInstance = Com_LoadLibrary( name, true );
		if( !svgame.hInstance ) return false;
	#else

	#endif

	// make local copy of engfuncs to prevent overwrite it with bots.dll
	Q_memcpy( &gpEngfuncs, &gEngfuncs, sizeof( gpEngfuncs ));

	GetEntityAPI = (APIFUNCTION)Com_GetProcAddress( svgame.hInstance, "GetEntityAPI" );
	GetEntityAPI2 = (APIFUNCTION2)Com_GetProcAddress( svgame.hInstance, "GetEntityAPI2" );
	GiveNewDllFuncs = (NEW_DLL_FUNCTIONS_FN)Com_GetProcAddress( svgame.hInstance, "GetNewDLLFunctions" );

	if( !GetEntityAPI && !GetEntityAPI2 )
	{
		Com_FreeLibrary( svgame.hInstance );
         		MsgDev( D_NOTE, "SV_LoadProgs: failed to get address of GetEntityAPI proc\n" );
		svgame.hInstance = NULL;
		return false;
	}

	#ifdef SURFACE_STATIC_GAME

	GiveFnptrsToDll = (GIVEFNPTRSTODLL)Com_GetProcAddress( svgame.hInstance, "GiveFnptrsToDll" );

	if( !GiveFnptrsToDll )
	{
		Com_FreeLibrary( svgame.hInstance );
		MsgDev( D_NOTE, "SV_LoadProgs: failed to get address of GiveFnptrsToDll proc\n" );
		svgame.hInstance = NULL;
		return false;
	}

	#endif

	GiveFnptrsToDll( &gpEngfuncs, svgame.globals );

	// get extended callbacks
	if( GiveNewDllFuncs )
	{
		version = NEW_DLL_FUNCTIONS_VERSION;
	
		if( !GiveNewDllFuncs( &svgame.dllFuncs2, &version ))
		{
			if( version != NEW_DLL_FUNCTIONS_VERSION )
				MsgDev( D_WARN, "SV_LoadProgs: new interface version %i should be %i\n", NEW_DLL_FUNCTIONS_VERSION, version );
			Q_memset( &svgame.dllFuncs2, 0, sizeof( svgame.dllFuncs2 ));
		}
	}

	version = INTERFACE_VERSION;

	if( GetEntityAPI2 )
	{
		if( !GetEntityAPI2( &svgame.dllFuncs, &version ))
		{
			MsgDev( D_WARN, "SV_LoadProgs: interface version %i should be %i\n", INTERFACE_VERSION, version );

			// fallback to old API
			if( !GetEntityAPI( &svgame.dllFuncs, version ))
			{
				Com_FreeLibrary( svgame.hInstance );
				MsgDev( D_ERROR, "SV_LoadProgs: couldn't get entity API\n" );
				svgame.hInstance = NULL;
				return false;
			}
		}
		else MsgDev( D_AICONSOLE, "SV_LoadProgs: ^2initailized extended EntityAPI ^7ver. %i\n", version );
	}
	else if( !GetEntityAPI( &svgame.dllFuncs, version ))
	{
		Com_FreeLibrary( svgame.hInstance );
		MsgDev( D_ERROR, "SV_LoadProgs: couldn't get entity API\n" );
		svgame.hInstance = NULL;
		return false;
	}

	if( !SV_InitPhysicsAPI( ))
	{
		MsgDev( D_WARN, "SV_LoadProgs: couldn't get physics API\n" );
	}

	// grab function SV_SaveGameComment
//	SV_InitSaveRestore ();

	svgame.globals->pStringBase = ""; // setup string base

	svgame.globals->maxEntities = GI->max_edicts;
	svgame.globals->maxClients = sv_maxclients->integer;
	svgame.edicts = Mem_Alloc( svgame.mempool, sizeof( edict_t ) * svgame.globals->maxEntities );
	svgame.numEntities = svgame.globals->maxClients + 1; // clients + world

	for( i = 0, e = svgame.edicts; i < svgame.globals->maxEntities; i++, e++ )
		e->free = true; // mark all edicts as freed

	Cvar_FullSet( "host_gameloaded", "1", CVAR_INIT );
	svgame.stringspool = Mem_AllocPool( "Server Strings" );

	// fire once
	MsgDev( D_INFO, "Dll loaded for mod %s\n", svgame.dllFuncs.pfnGetGameDescription( ));

	// all done, initialize game
	svgame.dllFuncs.pfnGameInit();

	Delta_Init ();

	// register custom encoders
	svgame.dllFuncs.pfnRegisterEncoders();

	return true;
}

void SV_UnloadProgs( void )
{
	SV_DeactivateServer ();
	Delta_Shutdown ();

	Mem_FreePool( &svgame.stringspool );

	svgame.dllFuncs.pfnGameShutdown ();

	// now we can unload cvars
	Cvar_FullSet( "host_gameloaded", "0", CVAR_INIT );

	// must unlink all game cvars,
	// before pointers on them will be lost...
	Cmd_ExecuteString( "@unlink\n", src_command );
	Cmd_Unlink( CMD_EXTDLL );

	Com_FreeLibrary( svgame.hInstance );
	Mem_FreePool( &svgame.mempool );
	Q_memset( &svgame, 0, sizeof( svgame ));
}