#ifndef SURFACE_CLIENT_H
#define SURFACE_CLIENT_H

#include <const.h>

#ifdef PRAGMA_ONCE
	#pragma once
#endif

typedef enum
{
	eTT_NET,	// Use network.c
	eTT_IPC,	// CreateFileMapping
	eTT_MEM,	// memcpy
} transmit_type_t;

// the client_t structure is wiped completely at every
// server map change
typedef struct
{
	int		timeoutcount;

	int		servercount;		// server identification for prespawns
	int		validsequence;		// this is the sequence number of the last good
						// world snapshot/update we got.  If this is 0, we can't
						// render a frame yet
	int		parsecount;		// server message counter
	int		parsecountmod;		// modulo with network window

	int		delta_sequence;		// acknowledged sequence number

	double		mtime[2];			// the timestamp of the last two messages


	int		last_incoming_sequence;

	uint		checksum;			// for catching cheater maps


	frame_t		frame;			// received from server
	int		predictcount;		// advances with next clientdata
	frame_t		frames[MULTIPLAYER_BACKUP];	// alloced on svc_serverdata

	double		time;			// this is the time value that the client
						// is rendering at.  always <= cls.realtime
						// a lerp point for other data
	double		oldtime;			// previous cl.time, time-oldtime is used
						// to decay light values and smooth step ups

	float		lerpFrac;			// interpolation value

	char		serverinfo[MAX_INFO_STRING];
	player_info_t	players[MAX_CLIENTS];
	event_state_t	events;

	// server state information
	int		playernum;
	int		maxclients;
	int		movemessages;
	transmit_type_t tr;

} remote_client_t;

typedef struct
{
	double		receivedtime;	// time message was received, or -1
	double		latency;
	double		time;		// server timestamp

	qboolean		valid;		// cleared if delta parsing was invalid
} frame_t;

typedef struct
{
	connstate_t	state;

	// screen rendering information
	float		disable_screen;		// showing loading plaque between levels
						// or changing rendering dlls
						// if time gets > 30 seconds ahead, break it
	int		disable_servercount;	// when we receive a frame and cl.servercount
						// > cls.disable_servercount, clear disable_screen
	
	int		framecount;
	int		quakePort;		// a 16 bit value that allows quake servers
						// to work around address translating routers
						// g-cont. this port allow many copies of engine in multiplayer game
	// connection information
	string		servername;		// name of server from original connect
	double		connect_time;		// for connection retransmits


	sizebuf_t		datagram;			// unreliable stuff. gets sent in CL_Move about cl_cmdrate times per second.
	byte		datagram_buf[NET_MAX_PAYLOAD];

	netchan_t		netchan;
	int		serverProtocol;		// in case we are doing some kind of version hack
	int		challenge;		// from the server to use for connecting

	float		packet_loss;
	double		packet_loss_recalc_time;

	float		nextcmdtime;		// when can we send the next command packet?                
	int		lastoutgoingcommand;	// sequence number of last outgoing command

	int		num_client_entities;	// cl.maxclients * CL_UPDATE_BACKUP * MAX_PACKET_ENTITIES
	int		next_client_entities;	// next client_entity to use
	packet_entity_t	*packet_entities;		// [num_client_entities]

	// download info
	int		downloadcount;
	int		downloadfileid;

	remote_client_t 	client;
} local_client_t;

typedef struct
{
	qboolean initialized;
	keydest_t		key_dest;
	byte		*mempool;			// client premamnent pool: edicts etc
	local_client_t 	local;
} first_client_t;

//
// cvars
//
extern convar_t	*cl_predict;
extern convar_t	*cl_smooth;
extern convar_t	*cl_showfps;
extern convar_t	*cl_envshot_size;
extern convar_t	*cl_timeout;
extern convar_t	*cl_nodelta;
extern convar_t	*cl_interp;
extern convar_t	*cl_crosshair;
extern convar_t	*cl_testlights;
extern convar_t	*cl_solid_players;
extern convar_t	*cl_idealpitchscale;
extern convar_t	*cl_allow_levelshots;
extern convar_t	*cl_lightstyle_lerping;
extern convar_t	*cl_draw_particles;
extern convar_t	*cl_levelshot_name;
extern convar_t	*cl_draw_beams;
extern convar_t	*scr_centertime;
extern convar_t	*scr_viewsize;
extern convar_t	*scr_download;
extern convar_t	*scr_loading;
extern convar_t	*scr_dark;	// start from dark
extern convar_t	*userinfo;

#endif // SURFACE_CLIENT_H