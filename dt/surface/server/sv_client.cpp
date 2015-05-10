#include <common.h>
#include "server.h"

/*
==================
SV_DisconnectClient

Disconnect client callback
==================
*/
void SV_DisconnectClient( CEdict *pClient )
{
	if( !pClient ) return;

	svgame.func.pfnClientDisconnect( pClient );

	if( pClient->pvPrivateData != NULL )
	{
		// NOTE: new interface can be missing
		svgame.func.pfnOnFreeEntPrivateData( pClient );

		// clear any dlls data but keep engine data
		Mem_Free( pClient->pvPrivateData );
		pClient->pvPrivateData = NULL;
	}
}

/*
==================
SV_FakeConnect

A connection request that came from the game module
==================
*/
edict_t *SV_FakeConnect( const char *netname )
{
	int		i, edictnum;
	char		userinfo[MAX_INFO_STRING];
	sv_client_t	temp, *cl, *newcl;
	edict_t		*ent;

	if( !netname ) netname = "";
	userinfo[0] = '\0';

	// force the IP key/value pair so the game can filter based on ip
	Info_SetValueForKey( userinfo, "ip", "127.0.0.1" );

	// find a client slot
	newcl = &temp;
	Q_memset( newcl, 0, sizeof( sv_client_t ));

	for( i = 0, cl = svs.clients; i < maxplayers->integer; i++, cl++ )
	{
		if( cl->state == cs_free )
		{
			newcl = cl;
			break;
		}
	}

	if( i == sv_maxclients->integer )
	{
		MsgDev( D_INFO, "SV_DirectConnect: rejected a connection.\n");
		return NULL;
	}

	// build a new connection
	// accept the new client
	// this is the only place a sv_client_t is ever initialized
	*newcl = temp;
	svs.currentPlayer = newcl;
	svs.currentPlayerNum = (newcl - svs.clients);
	edictnum = svs.currentPlayerNum + 1;

	if( newcl->frames )
		Mem_Free( newcl->frames );	// fakeclients doesn't have frames
	newcl->frames = NULL;

	ent = EDICT_NUM( edictnum );
	newcl->edict = ent;
	newcl->challenge = -1;		// fake challenge
	newcl->fakeclient = true;
	newcl->delta_sequence = -1;
	newcl->userid = g_userid++;		// create unique userid

	// get the game a chance to reject this connection or modify the userinfo
	if( !SV_ClientConnect( ent, userinfo ))
	{
		MsgDev( D_ERROR, "SV_DirectConnect: game rejected a connection.\n" );
		return NULL;
	}

	// parse some info from the info strings
	SV_UserinfoChanged( newcl, userinfo );

	MsgDev( D_NOTE, "Bot %i connecting with challenge %p\n", i, -1 );

	ent->v.flags |= FL_FAKECLIENT;	// mark it as fakeclient
	newcl->state = cs_spawned;
	newcl->lastmessage = host.realtime;	// don't timeout
	newcl->lastconnect = host.realtime;
	newcl->sendinfo = true;
	
	return ent;
}

/*
=====================
SV_ClientCconnect

QC code can rejected a connection for some reasons
e.g. ipban
=====================
*/
qboolean SV_ClientConnect( edict_t *ent, char *userinfo )
{
	qboolean	result = true;
	char	*pszName, *pszAddress;
	char	szRejectReason[MAX_INFO_STRING];

	// make sure we start with known default
	if( !sv.loadgame ) ent->v.flags = 0;
	szRejectReason[0] = '\0';

	pszName = Info_ValueForKey( userinfo, "name" );
	pszAddress = Info_ValueForKey( userinfo, "ip" );

	MsgDev( D_NOTE, "SV_ClientConnect()\n" );
	result = svgame.func.pfnClientConnect( ent, pszName, pszAddress, szRejectReason );
	if( szRejectReason[0] ) Info_SetValueForKey( userinfo, "rejmsg", szRejectReason );

	return result;
}

/*
=====================
SV_DropClient

Called when the player is totally leaving the server, either willingly
or unwillingly.  This is NOT called if the entire server is quiting
or crashing.
=====================
*/
void CSVClient::DropClient( )
{
	int	i;
	
	if( state == cs_zombie )
		return;	// already dropped

	// add the disconnect
	if( !fakeclient )
	{
		BF_WriteSvc( drop->curr_chan, svc_disconnect );
	}

	// let the game known about client state
	DisconnectClient( edict );

	fakeclient = false;
	hltv_proxy = false;
	state = cs_zombie; // become free in a few seconds
	name[0] = 0;

	if( frames )
		Mem_Free( frames );	// fakeclients doesn't have frames
	frames = NULL;

	// throw away any residual garbage in the channel.
	Netchan_Clear( netchan );

	// send notification to all other clients
	FullUpdate( &sv.reliable_datagram );

	// if this was the last client on the server, send a heartbeat
	// to the master so it is known the server is empty
	// send a heartbeat now so the master will get up to date info
	// if there is already a slot for this ip, reuse it
	for( i = 0; i < sv_maxclients->integer; i++ )
	{
		if( svs.clients[i].state >= cs_connected )
			break;
	}

	if( i == sv_maxclients->integer )
		svs.last_heartbeat = MAX_HEARTBEAT;
}

/*
==============================================================================

SVC COMMAND REDIRECT

==============================================================================
*/
void SV_BeginRedirect( netadr_t adr, int target, char *buffer, int buffersize, void (*flush))
{
	if( !target || !buffer || !buffersize || !flush )
		return;

	host.rd.target = target;
	host.rd.buffer = buffer;
	host.rd.buffersize = buffersize;
	host.rd.flush = flush;
	host.rd.address = adr;
	host.rd.buffer[0] = 0;
}

void SV_FlushRedirect( netadr_t adr, int dest, char *buf )
{
	if( svs.currentPlayer && svs.currentPlayer->fakeclient )
		return;

	switch( dest )
	{
	case RD_PACKET:
		Netchan_OutOfBandPrint( NS_SERVER, adr, "print\n%s", buf );
		break;
	case RD_CLIENT:
		if( !svs.currentPlayer ) return; // client not set
		BF_WriteByte( &svs.currentPlayer->netchan.message, svc_print );
		BF_WriteByte( &svs.currentPlayer->netchan.message, PRINT_HIGH );
		BF_WriteString( &svs.currentPlayer->netchan.message, buf );
		break;
	case RD_NONE:
		MsgDev( D_ERROR, "SV_FlushRedirect: %s: invalid destination\n", NET_AdrToString( adr ));
		break;
	}
}

void SV_EndRedirect( void )
{
	host.rd.flush( host.rd.address, host.rd.target, host.rd.buffer );

	host.rd.target = 0;
	host.rd.buffer = NULL;
	host.rd.buffersize = 0;
	host.rd.flush = NULL;
}

/*
===============
SV_GetClientIDString

Returns a pointer to a static char for most likely only printing.
===============
*/
const char *CSVClient::GetClientIDString( )
{
	static char	result[CS_SIZE];

	result[0] = '\0';

	if( !cl )
	{
		MsgDev( D_ERROR, "SV_GetClientIDString: invalid client\n" );
		return result;
	}

	if( cl->authentication_method == CA_HOTEL )
	{
		// probably some old compatibility code.
		Q_snprintf( result, sizeof( result ), "%s", htchan->stringauth );
	}
	else if( cl->authentication_method == CA_NET )
	{
		if( NET_IsLocalAddress( cl->netchan.remote_address ))
		{
			Q_strncpy( result, "ID_LOOPBACK", sizeof( result ));
		} else {

		}
	}
	else Q_strncpy( result, "UNKNOWN", sizeof( result ));

	return result;
}

/*
================
Rcon_Validate
================
*/
qboolean Rcon_Validate( void )
{
	if( !Q_strlen( rcon_password->string ))
		return false;
	if( Q_strcmp( Cmd_Argv( 1 ), rcon_password->string ))
		return false;
	return true;
}

/*
===============
SV_RemoteCommand

A client issued an rcon command.
Shift down the remaining args
Redirect all printfs
===============
*/
void SV_RemoteCommand( netadr_t from, sizebuf_t *msg )
{
	char		remaining[1024];
	static char	outputbuf[2048];
	int		i;

	MsgDev( D_INFO, "Rcon from %s:\n%s\n", NET_AdrToString( from ), BF_GetData( msg ) + 4 );
	SV_BeginRedirect( from, RD_PACKET, outputbuf, sizeof( outputbuf ) - 16, SV_FlushRedirect );

	if( Rcon_Validate( ))
	{
		remaining[0] = 0;
		for( i = 2; i < Cmd_Argc(); i++ )
		{
			Q_strcat( remaining, Cmd_Argv( i ));
			Q_strcat( remaining, " " );
		}
		Cmd_ExecuteString( remaining, src_command );
	}
	else MsgDev( D_ERROR, "Bad rcon_password.\n" );

	SV_EndRedirect();
}

/*
===================
SV_CalcPing

recalc ping on current client
===================
*/
int CSVClient::CalcPing( sv_client_t *cl )
{
	float		ping = 0;
	int		i, count;
	client_frame_t	*frame;

	// bots don't have a real ping
	if( fakeclient )
		return 5;

	count = 0;

	for( i = 0; i < SV_UPDATE_BACKUP; i++ )
	{
		frame = &frames[(netchan.incoming_acknowledged - (i + 1)) & SV_UPDATE_MASK];

		if( frame->raw_ping > 0 )
		{
			ping += frame->raw_ping;
			count++;
		}
	}

	if( !count )
		return 0;

	return (( ping / count ) * 1000 );
}

/*
===================
SV_FullClientUpdate

Writes all update values to a bitbuf
===================
*/
void CSVClient::FullUpdate( sizebuf_t *msg )
{
	char	info[MAX_INFO_STRING];
	int	i;	

	i = this - svs.clients;

	BF_WriteSvc( msg, svc_updateuserinfo );
	BF_WriteUBitLong( msg, i, MAX_CLIENT_BITS );

	if( name[0] )
	{
		BF_WriteOneBit( msg, 1 );

		Q_strncpy( info, userinfo, sizeof( info ));

		// remove server passwords, etc.
		Info_RemovePrefixedKeys( info, '_' );
		BF_WriteString( msg, info );
	}
	else BF_WriteOneBit( msg, 0 );
}

/*
===================
SV_RefreshUserinfo

===================
*/
void SV_RefreshUserinfo( void )
{
	int		i;
	CSVClient	*cl;

	for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
		if( cl->state >= cs_connected ) cl->sendinfo = true;
}

/*
===================
SV_IsPlayerIndex

===================
*/
qboolean SV_IsPlayerIndex( int idx )
{
	if( idx > 0 && idx <= maxplayers->integer )
		return true;
	return false;
}

/*
===================
SV_GetPlayerStats

This function and its static vars track some of the networking
conditions.  I haven't bothered to trace it beyond that, because
this fucntion sucks pretty badly.
===================
*/
void CSVClient::GetPlayerStats( int *ping, int *packet_loss )
{
	static int	last_ping[MAX_CLIENTS];
	static int	last_loss[MAX_CLIENTS];
	int		i;

	i = cl - svs.clients;

	if( cl->next_checkpingtime < host.realtime )
	{
		cl->next_checkpingtime = host.realtime + 2.0;
		last_ping[i] = SV_CalcPing( cl );
		last_loss[i] = cl->packet_loss;
	}

	if( ping ) *ping = last_ping[i];
	if( packet_loss ) *packet_loss = last_loss[i];
}

/*
===========
PutClientInServer

Called when a player connects to a server or respawns in
a deathmatch.
============
*/
void SV_PutClientInServer( edict_t *ent )
{
	CSVClient	*client;

	client = SV_ClientFromEdict( ent, true );
	ASSERT( client != NULL );

	if( !sv.loadgame )
	{	
		client->hltv_proxy = Q_atoi( Info_ValueForKey( client->userinfo, "hltv" )) ? true : false;

		if( client->hltv_proxy )
			ent->v.flags |= FL_PROXY;
		else ent->v.flags = 0;

		ent->v.netname = MAKE_STRING( client->name );

		// fisrt entering
		svgame.globals->time = sv.time;
		svgame.func.pfnClientPutInServer( ent );
	}

	if( sv_maxclients->integer == 1 ) // singleplayer profiler
		MsgDev( D_INFO, "level loaded at %.2f sec\n", Sys_DoubleTime() - svs.timestart );
}

/*
============================================================

CLIENT COMMAND EXECUTION

============================================================
*/
/*
================
SV_New_f

Sends the first message from the server to a connected client.
This will be sent on the initial connection and upon each server load.
================
*/
void CSVClient::New( )
{
	int	playernum;
	edict_t	*ent;

	if( state != cs_connected )
	{
		return;
	}

	playernum = cl - svs.clients;

	// send the serverdata
	BF_WriteSvc( curr_chan, svc_serverinfo );
	BF_WriteLong( curr_chan, PROTOCOL_VERSION );
	BF_WriteLong( curr_chan, svs.spawncount );
	BF_WriteLong( curr_chan, sv.checksum );
	BF_WriteByte( curr_chan, playernum );
	BF_WriteByte( curr_chan, svgame.globals->maxPlayers );
	BF_WriteWord( curr_chan, svgame.globals->maxEntities );
	BF_WriteString( curr_chan, sv.name );
	BF_WriteString( curr_chan, GI->gamefolder );

	// refresh userinfo on spawn
	SV_RefreshUserinfo();

	// game server
	if( sv.state == ss_active )
	{
		// set up the entity for the client
		ent = EDICT_NUM( playernum + 1 );
		cl->edict = ent;

		//BF_WriteSvc( curr_chan, net_signon );	
	}
}
