#include <common.h>
#include "server.h"

convar_t 	*mp_gamemode;

/*
===================
SV_CalcPings

Updates the cl->ping variables
===================
*/
void SV_CalcPings( void )
{
	sv_client_t	*cl;
	int		i, j;
	int		total, count;

	if( !svs.clients ) return;

	// clamp fps counter
	for( i = 0; i < sv_maxclients->integer; i++ )
	{
		cl = &svs.clients[i];

		if( cl->state != cs_spawned || cl->fakeclient )
			continue;

		total = count = 0;

		for( j = 0; j < (SV_UPDATE_BACKUP / 2); j++ )
		{
			client_frame_t	*frame;

			frame = &cl->frames[(cl->netchan.incoming_acknowledged - (j + 1)) & SV_UPDATE_MASK];
			if( frame->latency > 0 )
			{
				count++;
				total += frame->latency;
			}
		}

		if( !count ) cl->ping = 0;
		else cl->ping = total / count;
	}
}

/*
=================
SV_ReadPackets
=================
*/
void SV_ReadPackets( void )
{
	CSVClient	*cl;
	int		i, qport, curSize;
	
	for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
	{
		sizebuf_t *ht_message;

		if ( cl->curr_chan != &cl->htchar.bf )
			continue;
		
		while ( ( htbuff = Ht_GetPacket( &curSize ) != NULL ) )
		{
			cl.ExecuteMessage( ht_message );
			Ht_WorkResident( cl->htchan->htadr );
		}
	}

	while( NET_GetPacket( NS_SERVER, &net_from, net_message_buffer, &curSize ))
	{
		BF_Init( &net_message, "ClientPacket", net_message_buffer, curSize );

		// check for connectionless packet (0xffffffff) first
		if( BF_GetMaxBytes( &net_message ) >= 4 && *(int *)net_message.pData == -1 )
		{
			SV_ConnectionlessPacket( net_from, &net_message );
			continue;
		}

		// read the qport out of the message so we can fix up
		// stupid address translating routers
		BF_Clear( &net_message );
		BF_ReadLong( &net_message );	// sequence number
		BF_ReadLong( &net_message );	// sequence number
		qport = (int)BF_ReadShort( &net_message ) & 0xffff;

		// check for packets from connected clients
		for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
		{
			if( cl->state == cs_free || cl->fakeclient )
				continue;

			if( !NET_CompareBaseAdr( net_from, cl->netchan.remote_address ))
				continue;

			if( cl->netchan.qport != qport )
				continue;

			if( cl->netchan.remote_address.port != net_from.port )
			{
				MsgDev( D_INFO, "SV_ReadPackets: fixing up a translated port\n");
				cl->netchan.remote_address.port = net_from.port;
			}

			if( Netchan_Process( &cl->netchan, &net_message ))
			{	
				cl->send_message = true; // reply at end of frame

				// this is a valid, sequenced packet, so process it
				if( cl->state != cs_zombie )
				{
					cl->lastmessage = host.realtime; // don't timeout
					SV_ExecuteClientMessage( cl, &net_message );
					svgame.globals->frametime = host.frametime;
					svgame.globals->time = sv.time;
				}
			}

			// fragmentation/reassembly sending takes priority over all game messages, want this in the future?
			if( Netchan_IncomingReady( &cl->netchan ))
			{
				if( Netchan_CopyNormalFragments( &cl->netchan, &net_message ))
				{
					BF_Clear( &net_message );
					cl.ExecuteMessage( &net_message );
				}

				/*if( Netchan_CopyFileFragments( &cl->netchan, &net_message ))
				{
					SV_ProcessFile( cl, cl->netchan.incomingfilename );
				}*/
				
				// SV_TorrentProcess( cl );
			}
			break;
		}

		if( i != sv_maxclients->integer )
			continue;
	}
	
}

/*
================
SV_HasActivePlayers

returns true if server have spawned players
================
*/
qboolean SV_HasActivePlayers( void )
{
	int	i;

	// server inactive
	if( !svs.clients ) return false;

	for( i = 0; i < sv_maxclients->integer; i++ )
	{
		if( svs.clients[i].state == cs_spawned )
			return true;
	}
	return false;
}

/*
==================
SV_CheckTimeouts

If a packet has not been received from a client for timeout->value
seconds, drop the conneciton.  Server frames are used instead of
realtime to avoid dropping the local client while debugging.

When a client is normally dropped, the sv_client_t goes into a zombie state
for a few seconds to make sure any final reliable message gets resent
if necessary
==================
*/
void SV_CheckTimeouts( void )
{
	sv_client_t	*cl;
	float		droppoint;
	float		zombiepoint;
	int		i, numclients = 0;

	droppoint = host.realtime - timeout->value;
	zombiepoint = host.realtime - zombietime->value;

	for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
	{
		if( cl->state >= cs_connected )
		{
			if( cl->edict && !( cl->edict->v.flags & (FL_SPECTATOR|FL_FAKECLIENT)))
				numclients++;
                    }

		// fake clients do not timeout
		if( cl->fakeclient ) cl->lastmessage = host.realtime;

		// message times may be wrong across a changelevel
		if( cl->lastmessage > host.realtime )
			cl->lastmessage = host.realtime;

		if( cl->state == cs_zombie && cl->lastmessage < zombiepoint )
		{
			cl->state = cs_free; // can now be reused
			continue;
		}

		if(( cl->state == cs_connected || cl->state == cs_spawned ) && cl->lastmessage < droppoint )
		{
			SV_BroadcastPrintf( PRINT_HIGH, "%s timed out\n", cl->name );
			SV_DropClient( cl ); 
			cl->state = cs_free; // don't bother with zombie state
		}
	}

	if( !numclients )
	{
		// nobody left, unpause the server
		SV_TogglePause( "Pause released since no players are left." );
	}
}

/*
================
SV_PrepWorldFrame

This has to be done before the world logic, because
player processing happens outside RunWorldFrame
================
*/
void SV_PrepWorldFrame( void )
{
	edict_t	*ent;
	int	i;

	for( i = 1; i < svgame.numEntities; i++ )
	{
		ent = EDICT_NUM( i );
		if( ent->free ) continue;

		ent->v.effects &= ~(EF_NOINTERP);
	}
}

/*
=================
SV_RunGameFrame
=================
*/
void SV_RunGameFrame( void )
{
	// if( !SV_IsSimulating( )) return;

	// SV_Physics();

	sv.time += host.frametime;
}

/*
==================
Host_ServerFrame

==================
*/
void Host_ServerFrame( void )
{
	// if server is not active, do nothing
	if( !svs.initialized ) return;

	svgame.globals->frametime = host.frametime;

	// check timeouts
	SV_CheckTimeouts ();

	// check clients timewindow
	SV_CheckCmdTimes ();

	// read packets from clients
	SV_ReadPackets ();

	// update ping based on the last known frame from all clients
	SV_CalcPings ();

	// refresh serverinfo on the client side
	SV_UpdateServerInfo ();

	// refresh physic movevars on the client side
	SV_UpdateMovevars ( false );

	// let everything in the world think and move
	SV_RunGameFrame ();
		
	// send messages back to the clients that had packets read this frame
	SV_SendClientMessages ();

	// clear edict flags for next frame
	SV_PrepWorldFrame ();

	// send a heartbeat to the master if needed
	Master_Heartbeat ();
}

//============================================================================

/*
=================
Master_Add
=================
*/
void Master_Add( void )
{
	netadr_t	adr;

	NET_Config( true ); // allow remote

	if( !NET_StringToAdr( MASTERSERVER_ADR, &adr ))
		MsgDev( D_INFO, "Can't resolve adr: %s\n", MASTERSERVER_ADR );

	NET_SendPacket( NS_SERVER, 2, "\x4D\xFF", adr );
}

/*
================
Master_Heartbeat

Send a message to the master every few minutes to
let it know we are alive, and log information
================
*/
void Master_Heartbeat( void )
{
	if( !public_server->integer || sv_maxclients->integer == 1 )
		return; // only public servers send heartbeats

	// check for time wraparound
	if( svs.last_heartbeat > host.realtime )
		svs.last_heartbeat = host.realtime;

	if(( host.realtime - svs.last_heartbeat ) < HEARTBEAT_SECONDS )
		return; // not time to send yet

	svs.last_heartbeat = host.realtime;

	Master_Add();
}

/*
=================
Master_Shutdown

Informs all masters that this server is going down
=================
*/
void Master_Shutdown( void )
{
}

/*
==================
SV_FinalMessage

Used by SV_Shutdown to send a final message to all
connected clients before the server goes down.  The messages are sent immediately,
not just stuck on the outgoing message list, because the server is going
to totally exit after returning from this function.
==================
*/
void SV_FinalMessage( char *message, qboolean reconnect )
{
	sv_client_t	*cl;
	byte		msg_buf[1024];
	sizebuf_t		msg;
	int		i;
	
	BF_Init( &msg, "FinalMessage", msg_buf, sizeof( msg_buf ));
	BF_WriteByte( &msg, svc_print );
	BF_WriteByte( &msg, PRINT_HIGH );
	BF_WriteString( &msg, va( "%s\n", message ));

	if( reconnect )
	{
		BF_WriteSvc( &msg, svc_changing );
	}
	else
	{
		BF_WriteSvc( &msg, svc_disconnect );
	}

	// send it twice
	// stagger the packets to crutch operating system limited buffers
	for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
		if( cl->state >= cs_connected && !cl->fakeclient )
			Netchan_Transmit( &cl->netchan, BF_GetNumBytesWritten( &msg ), BF_GetData( &msg ));

	for( i = 0, cl = svs.clients; i < sv_maxclients->integer; i++, cl++ )
		if( cl->state >= cs_connected && !cl->fakeclient )
			Netchan_Transmit( &cl->netchan, BF_GetNumBytesWritten( &msg ), BF_GetData( &msg ));
}

/*
================
SV_Shutdown

Called when each game quits,
before Sys_Quit or Sys_Error
================
*/
void SV_Shutdown( qboolean reconnect )
{
	// already freed
	if( !SV_Active( )) return;

	if( host.type == HOST_DEDICATED ) MsgDev( D_INFO, "SV_Shutdown: %s\n", host.finalmsg );
	if( svs.clients ) SV_FinalMessage( host.finalmsg, reconnect );

	Master_Shutdown();

	if( !reconnect ) SV_UnloadProgs ();
	else SV_DeactivateServer ();

	// free current level
	Q_memset( &sv, 0, sizeof( sv ));
	Host_SetServerState( sv.state );

	// free server static data
	if( svs.clients )
	{
		Z_Free( svs.clients );
		svs.clients = NULL;
	}

	if( svs.baselines )
	{
		Z_Free( svs.baselines );
		svs.baselines = NULL;
	}

	if( svs.packet_entities )
	{
		Z_Free( svs.packet_entities );
		svs.packet_entities = NULL;
		svs.num_client_entities = 0;
		svs.next_client_entities = 0;
	}

	svs.initialized = false;
}