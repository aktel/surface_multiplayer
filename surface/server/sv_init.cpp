#include "common.h"
#include "server.h"

int SV_UPDATE_BACKUP = SINGLEPLAYER_BACKUP;

server_static_t	svs;	// persistant server info
svgame_static_t	svgame;	// persistant game info
server_t		sv;	// local server

/*
================
SV_CreateBaseline

Entity baselines are used to compress the update messages
to the clients -- only the fields that differ from the
baseline will be transmitted
================
*/
void SV_CreateBaseline( void )
{
	CEdict	*pEdict;
	int	e;	

	for( e = 0; e < svgame.numEntities; e++ )
	{
		pEdict = EDICT_NUM( e );
		if( !SV_IsValidEdict( pEdict )) continue;
		SV_BaselineForEntity( pEdict );
	}

	// create the instanced baselines
	svgame.func.pfnCreateInstancedBaselines();
}

/*
================
SV_FreeOldEntities

remove immediate entities
================
*/
void SV_FreeOldEntities( void )
{
	CEdict	*ent;
	int	i;

	// at end of frame kill all entities which supposed to it 
	for( i = svgame.globals->maxClients + 1; i < svgame.numEntities; i++ )
	{
		ent = EDICT_NUM( i );
		if( ent->free ) continue;

		if( ent->v.flags & FL_KILLME )
			SV_FreeEdict( ent );
	}

	// decrement svgame.numEntities if the highest number entities died
	for( ; EDICT_NUM( svgame.numEntities - 1 )->free; svgame.numEntities-- );
}

/*
================
SV_ActivateServer

activate server on changed map, run physics
================
*/
void SV_ActivateServer( void )
{
	int	i, numFrames;

	if( !svs.initialized )
		return;

	// Activate the DLL server code
	svgame.func.pfnServerActivate( svgame.edicts, svgame.numEntities, svgame.globals->maxClients );

	// create a baseline for more efficient communications
	SV_CreateBaseline();

	// send serverinfo to all connected clients
	for( i = 0; i < sv_maxclients->integer; i++ )
	{
		if( svs.clients[i].state >= cs_connected )
		{
			Netchan_Clear( &svs.clients[i].netchan );
			svs.clients[i].delta_sequence = -1;
		}
	}

	// invoke to refresh all movevars
	Q_memset( &svgame.oldmovevars, 0, sizeof( movevars_t ));
	svgame.globals->changelevel = false; // changelevel ends here

	// setup hostflags
	sv.hostflags = 0;

	// tell what kind of server has been started.
	if( svgame.globals->maxClients > 1 )
	{
		MsgDev( D_INFO, "%i player server started\n", svgame.globals->maxClients );
		Cvar_Reset( "clockwindow" );
	}
	else
	{
		// clear the ugly moving delay in singleplayer
		Cvar_SetFloat( "clockwindow", 0.0f );
		MsgDev( D_INFO, "Game started\n" );
	}

	sv.state = ss_active;
	physinfo->modified = true;
	sv.changelevel = false;
	sv.paused = false;

	Host_SetServerState( sv.state );

	if( sv_maxclients->integer > 1 && public_server->integer )
	{
		MsgDev( D_INFO, "Add your server, to master server list\n" );
		Master_Add( );
	}
}

/*
================
SV_DeactivateServer

deactivate server, free edicts, strings etc
================
*/
void SV_DeactivateServer( void )
{
	int	i;

	if( !svs.initialized || sv.state == ss_dead )
		return;

	sv.state = ss_dead;

	SV_FreeEdicts ();

	Mem_EmptyPool( svgame.stringspool );

	svgame.func.pfnServerDeactivate();

	for( i = 0; i < sv_maxclients->integer; i++ )
	{
		// release client frames
		if( svs.clients[i].frames )
			Mem_Free( svs.clients[i].frames );
		svs.clients[i].frames = NULL;
	}

	svgame.globals->maxEntities = GI->max_edicts;
	svgame.globals->maxClients = sv_maxclients->integer;
	svgame.numEntities = svgame.globals->maxClients + 1; // clients + world
}