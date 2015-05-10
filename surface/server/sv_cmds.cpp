#include "common.h"
#include "server.h"

/*
=================
SV_ClientPrintf

Sends text across to be displayed if the level passes
=================
*/
void SV_ClientPrintf( sv_client_t *cl, int level, char *fmt, ... )
{
	va_list	argptr;
	char	string[MAX_SYSPATH];

	if( level < cl->messagelevel || cl->fakeclient )
		return;
	
	va_start( argptr, fmt );
	Q_vsprintf( string, fmt, argptr );
	va_end( argptr );
	
	BF_WriteSvc( cl->curr_chan, svc_print );
	BF_WriteByte( cl->curr_chan, level );
	BF_WriteString( cl->curr_chan, string );
}

/*
=================
SV_BroadcastPrintf

Sends text to all active clients
=================
*/
void SV_BroadcastPrintf( int level, char *fmt, ... )
{
	char		string[MAX_SYSPATH];
	va_list		argptr;
	sv_client_t	*cl;
	int		i;

	if( !sv.state ) return;

	va_start( argptr, fmt );
	Q_vsprintf( string, fmt, argptr );
	va_end( argptr );
	
	// echo to console
	if( host.type == HOST_DEDICATED ) Msg( "%s", string );

	for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
	{
		if( level < cl->messagelevel ) continue;
		if( cl->state != cs_spawned ) continue;
		if( cl->fakeclient ) continue;

		BF_WriteSvc( cl->curr_chan, svc_print );
		BF_WriteByte( cl->curr_chan, level );
		BF_WriteString( cl->curr_chan, string );
	}
}

/*
=================
SV_BroadcastCommand

Sends text to all active clients
=================
*/
void SV_BroadcastCommand( char *fmt, ... )
{
	va_list	argptr;
	char	string[MAX_SYSPATH];
	
	if( !sv.state ) return;
	va_start( argptr, fmt );
	Q_vsprintf( string, fmt, argptr );
	va_end( argptr );

	BF_WriteSvc( &sv.reliable_datagram, svc_stufftext );
	BF_WriteString( &sv.reliable_datagram, string );
}


/*
==================
SV_SetPlayer

Sets sv_client and sv_player to the player with idnum Cmd_Argv(1)
==================
*/
qboolean SV_SetPlayer( void )
{
	char		*s;
	CSVClient	*cl;
	int		i, idnum;

	if( !svs.clients )
	{
		Msg( "^3no server running.\n" );
		return false;
    }

	if( sv_maxclients->integer == 1 || Cmd_Argc() < 2 )
	{
		// special case for local client
		svs.currentPlayer = svs.clients;
		svs.currentPlayerNum = 0;
		return true;
	}

	s = Cmd_Argv( 1 );

	// numeric values are just slot numbers
	if( Q_isdigit( s ) || (s[0] == '-' && Q_isdigit( s + 1 )))
	{
		idnum = Q_atoi( s );
		if( idnum < 0 || idnum >= sv_maxclients->integer )
		{
			Msg( "Bad client slot: %i\n", idnum );
			return false;
		}

		svs.currentPlayer = &svs.clients[idnum];
		svs.currentPlayerNum = idnum;

		if( !svs.currentPlayer->state )
		{
			Msg( "Client %i is not active\n", idnum );
			return false;
		}
		return true;
	}

	// check for a name match
	for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
	{
		if( !cl->state ) continue;
		if( !Q_strcmp( cl->name, s ))
		{
			svs.currentPlayer = cl;
			svs.currentPlayerNum = (cl - svs.clients);
			return true;
		}
	}

	Msg( "Userid %s is not on the server\n", s );
	svs.currentPlayer = NULL;
	svs.currentPlayerNum = 0;

	return false;
}

/*
==============
SV_EndGame_f

==============
*/
void SV_EndGame_f( void )
{
	Host_EndGame( Cmd_Argv( 1 ));
}

/*
==============
SV_KillGame_f

==============
*/
void SV_KillGame_f( void )
{
	Host_EndGame( "The End" );
}

/*
==============
SV_Load_f

==============
*/
void SV_Load_f( void )
{
	if( Cmd_Argc() != 2 )
	{
		Msg( "Usage: load <savename>\n" );
		return;
	}
	SV_LoadGame( Cmd_Argv( 1 ));
}

/*
==============
SV_QuickLoad_f

==============
*/
void SV_QuickLoad_f( void )
{
	Cbuf_AddText( "echo Quick Loading...; wait; load quick" );
}

/*
==============
SV_Save_f

==============
*/
void SV_Save_f( void )
{
	const char *name;

	switch( Cmd_Argc() )
	{
	case 1: name = "new"; break;
	case 2: name = Cmd_Argv( 1 ); break;
	default:
		Msg( "Usage: save <savename>\n" );
		return;
	}

	SV_SaveGame( name );
}

/*
==============
SV_DeleteSave_f

==============
*/
void SV_DeleteSave_f( void )
{
	if( Cmd_Argc() != 2 )
	{
		Msg( "Usage: delsave <name>\n" );
		return;
	}

	// delete save and saveshot
	FS_Delete( va( "save/%s.sav", Cmd_Argv( 1 )));
	//FS_Delete( va( "save/%s.bmp", Cmd_Argv( 1 )));
}

/*
==============
SV_AutoSave_f

==============
*/
void SV_AutoSave_f( void )
{
	if( Cmd_Argc() != 1 )
	{
		Msg( "Usage: autosave\n" );
		return;
	}

	SV_SaveGame( "autosave" );
}

/*
==================
SV_Reload_f

continue from latest savedgame
==================
*/
void SV_Reload_f( void )
{
	const char	*save;
	string		loadname;
	
	if( sv.state != ss_active || sv.background )
		return;

	save = SV_GetLatestSave();

	if( save )
	{
		FS_FileBase( save, loadname );
		Cbuf_AddText( va( "load %s\n", loadname ));
	}
	else Cbuf_AddText( "newgame\n" ); // begin new game
}

/*
==================
SV_Kick_f

Kick a user off of the server
==================
*/
void SV_Kick_f( void )
{
	if( Cmd_Argc() != 2 )
	{
		Msg( "Usage: kick <userid>\n" );
		return;
	}

	if( !svs.clients || sv.background )
	{
		Msg( "^3no server running.\n" );
		return;
	}

	if( !SV_SetPlayer( )) return;

	SV_BroadcastPrintf( PRINT_HIGH, "%s was kicked\n", svs.currentPlayer->name );
	SV_ClientPrintf( svs.currentPlayer, PRINT_HIGH, "You were kicked from the game\n" );
	SV_DropClient( svs.currentPlayer );

	// min case there is a funny zombie
	svs.currentPlayer->lastmessage = host.realtime;
}


/*
================
SV_Status_f
================
*/
void SV_Status_f( void )
{
	int		i;
	CSVClient	*cl;

	if( !svs.clients || sv.background )
	{
		Msg( "^3no server running.\n" );
		return;
	}

	Msg( "map: %s\n", sv.name );
	Msg( "num score ping    name            lastmsg address               port \n" );
	Msg( "--- ----- ------- --------------- ------- --------------------- ------\n" );

	for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
	{
		int	j, l, ping;
		char	*s;

		if( !cl->state ) continue;

		Msg( "%3i ", i );
		Msg( "%5i ", (int)cl->edict->v.frags );

		if( cl->state == cs_connected ) Msg( "Connect" );
		else if( cl->state == cs_zombie ) Msg( "Zombie " );
		else if( cl->fakeclient ) Msg( "Bot   " );
		else
		{
			ping = cl->ping < 9999 ? cl->ping : 9999;
			Msg( "%7i ", ping );
		}

		Msg( "%s", cl->name );
		l = 24 - Q_strlen( cl->name );
		for( j = 0; j < l; j++ ) Msg( " " );
		Msg( "%g ", ( host.realtime - cl->lastmessage ));
		s = NET_BaseAdrToString( cl->netchan.remote_address );
		Msg( "%s", s );
		l = 22 - Q_strlen( s );
		for( j = 0; j < l; j++ ) Msg( " " );
		Msg( "%5i", cl->netchan.qport );
		Msg( "\n" );
	}
	Msg( "\n" );
}

/*
==================
SV_ConSay_f
==================
*/
void SV_ConSay_f( void )
{
	char		*p, text[MAX_SYSPATH];
	sv_client_t	*client;
	int		i;

	if( Cmd_Argc() < 2 ) return;

	if( !svs.clients || sv.background )
	{
		Msg( "^3no server running.\n" );
		return;
	}

	Q_strncpy( text, "console: ", MAX_SYSPATH );
	p = Cmd_Args();

	if( *p == '"' )
	{
		p++;
		p[Q_strlen(p) - 1] = 0;
	}

	Q_strncat( text, p, MAX_SYSPATH );

	for( i = 0, client = svs.clients; i < sv_maxclients->integer; i++, client++ )
	{
		if( client->state != cs_spawned )
			continue;

		SV_ClientPrintf( client, PRINT_CHAT, "%s\n", text );
	}
}

/*
==================
SV_Heartbeat_f
==================
*/
void SV_Heartbeat_f( void )
{
	svs.last_heartbeat = MAX_HEARTBEAT;
}

/*
===========
SV_ServerInfo_f

Examine serverinfo string
===========
*/
void SV_ServerInfo_f( void )
{
	Msg( "Server info settings:\n" );
	Info_Print( Cvar_Serverinfo( ));
}

/*
===========
SV_ClientInfo_f

Examine all a users info strings
===========
*/
void SV_ClientInfo_f( void )
{
	if( Cmd_Argc() != 2 )
	{
		Msg( "Usage: clientinfo <userid>\n" );
		return;
	}

	if( !SV_SetPlayer( )) return;
	Msg( "userinfo\n" );
	Msg( "--------\n" );
	Info_Print( svs.currentPlayer->userinfo );

}

/*
===============
SV_KillServer_f

Kick everyone off, possibly in preparation for a new game
===============
*/
void SV_KillServer_f( void )
{
	if( !svs.initialized ) return;
	Q_strncpy( host.finalmsg, "Server was killed", MAX_STRING );
	SV_Shutdown( false );
	NET_Config ( false ); // close network sockets
}


/*
===============
SV_EdictsInfo_f

===============
*/
void SV_EdictsInfo_f( void )
{
	int	active;

	if( sv.state != ss_active )
	{
		Msg( "^3no server running.\n" );
		return;
	}

	active = pfnNumberOfEntities(); 
	Msg( "%5i edicts is used\n", active );
	Msg( "%5i edicts is free\n", svgame.globals->maxEntities - active );
	Msg( "%5i total\n", svgame.globals->maxEntities );
}