#ifndef SURFACE_SERVER_H
#define SURFACE_SERVER_H

#include "common.h"

#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "netchan.h"
#include "edict.h"
#include "serverinterface.h"
#include "protocol.h"
#include "hotel.h"

class CSVClient
{
public:
	char		name[32];			// extracted from userinfo, color string allowed

	char		userinfo[MAX_INFO_STRING];	// name, etc (received from client)

	netchan_t 		chan;

	qboolean		lag_compensation;		// enable lag compensation
	qboolean		hltv_proxy;		// this is spectator proxy (hltv)	

	int		chokecount;         	// number of messages rate supressed
	int		delta_sequence;		// -1 = no compression.

	double		next_messagetime;		// time when we should send next world state update  
	double		cl_updaterate;		// default time to wait for next message
	double		next_checkpingtime;		// time to send all players pings to client
	double		timebase;	

	int		packet_loss;
	float		latency;
	float		ping;

	// the datagram is written to by sound calls, prints, temp ents, etc.
	// it can be harmlessly overflowed.
	sizebuf_t		datagram;
	byte		datagram_buf[NET_MAX_PAYLOAD];

	sizebuf_t	*curr_chan;		// Pointer from chan.message or htchan.message

	htchan_t 	htchan;

	double		lastmessage;		// time when packet was last received
	double		lastconnect;

	int		challenge;		// challenge of this user, randomly generated
	int		userid;			// identifying number on server
	int		authentication_method;

	union
	{
		uint 		edicts_leght;
		CEdict		*allow_input;
	}

	qboolean 	fakeclient;
	qboolean 	isPlayer;

	void 	SendDisconnectMessage();
	int 	CalcPacketLoss();
	void 	WriteFrameToClient();
	void	ShouldUpdatePing();

	const char *GetClientIDString();

	//void DropClient();
};

class CSVGame
{
public:
	void 	*hInstance;

	int 	numEntities;

	// CServerVars		*svv;

	gamefunc_t		func;

	byte 	*mempool;
	byte    *stringspool;

	CSaveRestore	Save;
};

class CServerState
{
public:
	qboolean 		initialized; // sv_init has completed
	double		timestart;	// just for profiling

	CSVClient	*clients;
	CSVClient	*currentPlayer;
	int 	currentPlayerNum;
	int 	num_client_entities;

	CPacketEntities		*packet_entities;
	CPacketEntities		*firststate;
};

class CClientFrame
{
public:
	double		senttime;
	float		raw_ping;
	float		latency;

	clientdata_t	clientdata;
	weapon_data_t	weapondata[MAX_WEAPONS];
	weapon_data_t	oldweapondata[MAX_WEAPONS];	// g-cont. The fucking Cry Of Fear a does corrupting memory after the weapondata!!!

	int  		num_entities;
	int  		first_entity;		// into the circular sv_packet_entities[]
};

extern convar_t 	*sv_allowdownload;
extern convar_t 	*sv_allowupload;
extern convar_t 	*sv_torrentserver;
extern convar_t 	*sv_cheats;
extern convar_t		*sv_servertype;
extern convar_t 	*rcon_password;
extern convar_t 	*hostname;
extern convar_t 	*maxplayers;
extern convar_t 	*sv_maxclients;	// value is not less on maxplayers
extern convar_t		*sv_tags;
//extern convar_t 	*sv_requiredmodules;
extern convar_t 	*mp_gamemode;
extern convar_t 	*sv_unlag;
extern convar_t 	*sv_maxunlag;
extern convar_t 	*sv_force_prediction;
extern convar_t		*sv_masterservers;

//===========================================================
//
// sv_main.c
//
void SV_FinalMessage( char *message, qboolean reconnect );
void SV_DropClient( sv_client_t *drop );
int SV_CalcPacketLoss( sv_client_t *cl );
void SV_ExecuteUserCommand (char *s);
void SV_InitOperatorCommands( void );
void SV_KillOperatorCommands( void );
void SV_UserinfoChanged( sv_client_t *cl, const char *userinfo );
void SV_PrepWorldFrame( void );
void SV_ProcessFile( sv_client_t *cl, char *filename );
void SV_SendResourceList( sv_client_t *cl );
void Master_Heartbeat( void );
void Master_Packet( void );

//
// sv_init.c
//
void SV_InitGame( void );
void SV_ActivateServer( void );
void SV_DeactivateServer( void );
qboolean SV_SpawnServer( const char *server, const char *startspot );

//
// sv_send.c
//
void SV_SendClientMessages( void );
void SV_ClientPrintf( sv_client_t *cl, int level, char *fmt, ... );
void SV_BroadcastPrintf( int level, char *fmt, ... );
void SV_BroadcastCommand( char *fmt, ... );

//
// sv_client.c
//
char *SV_StatusString( void );
void SV_RefreshUserinfo( void );
void SV_GetChallenge( netadr_t from );
void SV_DirectConnect( netadr_t from );
void SV_TogglePause( const char *msg );
void SV_PutClientInServer( edict_t *ent );
const char *SV_GetClientIDString( sv_client_t *cl );
void SV_GetPlayerStats( sv_client_t *cl, int *ping, int *packet_loss );
qboolean SV_ClientConnect( edict_t *ent, char *userinfo );
void SV_ClientThink( sv_client_t *cl, usercmd_t *cmd );
void SV_ExecuteClientMessage( sv_client_t *cl, sizebuf_t *msg );
void SV_ConnectionlessPacket( netadr_t from, sizebuf_t *msg );
edict_t *SV_FakeConnect( const char *netname );
 void SV_ExecuteClientCommand( sv_client_t *cl, char *s );
void SV_RunCmd( sv_client_t *cl, usercmd_t *ucmd, int random_seed );
qboolean SV_IsPlayerIndex( int idx );
void SV_InitClientMove( void );
void SV_UpdateServerInfo( void );

//
// sv_cmds.c
//
void SV_Status_f( void );
void SV_Newgame_f( void );

//
// sv_custom.c
//
void SV_SendResources( sizebuf_t *msg );
int SV_TransferConsistencyInfo( void );

//
// sv_frame.c
//
void SV_WriteFrameToClient( sv_client_t *client, sizebuf_t *msg );
void SV_BuildClientFrame( sv_client_t *client );
void SV_InactivateClients( void );
void SV_SendMessagesToAll( void );
void SV_SkipUpdates( void );

//
// sv_game.c
//
qboolean SV_LoadProgs( const char *name );
void SV_UnloadProgs( void );
void SV_FreeEdicts( void );
edict_t *SV_AllocEdict( void );
void SV_FreeEdict( edict_t *pEdict );
void SV_InitEdict( edict_t *pEdict );
const char *SV_ClassName( const edict_t *e );
void SV_SetModel( edict_t *ent, const char *name );
void SV_CopyTraceToGlobal( trace_t *trace );
void SV_SetMinMaxSize( edict_t *e, const float *min, const float *max );
edict_t* SV_FindEntityByString( edict_t *pStartEdict, const char *pszField, const char *pszValue );
void SV_PlaybackEventFull( int flags, const edict_t *pInvoker, word eventindex, float delay, float *origin,
	float *angles, float fparam1, float fparam2, int iparam1, int iparam2, int bparam1, int bparam2 );
void SV_PlaybackReliableEvent( sizebuf_t *msg, word eventindex, float delay, event_args_t *args );
void SV_BaselineForEntity( edict_t *pEdict );
void SV_WriteEntityPatch( const char *filename );
char *SV_ReadEntityScript( const char *filename, int *flags );
float SV_AngleMod( float ideal, float current, float speed );
void SV_SpawnEntities( const char *mapname, char *entities );
edict_t* SV_AllocPrivateData( edict_t *ent, string_t className );
string_t SV_AllocString( const char *szValue );
string_t SV_MakeString( const char *szValue );
const char *SV_GetString( string_t iString );
sv_client_t *SV_ClientFromEdict( const edict_t *pEdict, qboolean spawned_only );
void SV_StartSound( edict_t *ent, int chan, const char *sample, float vol, float attn, int flags, int pitch );
void SV_CreateStaticEntity( struct sizebuf_s *msg, sv_static_entity_t *ent );
edict_t* pfnPEntityOfEntIndex( int iEntIndex );
int pfnIndexOfEdict( const edict_t *pEdict );
int pfnNumberOfEntities( void );
void SV_RestartStaticEnts( void );

inline CEdict *SV_EDICT_NUM( int n, const char * file, const int line )
{
	if((n >= 0) && (n < svgame.globals->maxEntities))
		return svgame.edicts + n;
	Host_Error( "SV_EDICT_NUM: bad number %i (called at %s:%i)\n", n, file, line );
	return NULL;	
}

//
// sv_save.c
//
void SV_ClearSaveDir( void );
void SV_SaveGame( const char *pName );
qboolean SV_LoadGame( const char *pName );
void SV_ChangeLevel( qboolean loadfromsavedgame, const char *mapname, const char *start );
int SV_LoadGameState( char const *level, qboolean createPlayers );
void SV_LoadAdjacentEnts( const char *pOldLevel, const char *pLandmarkName );
const char *SV_GetLatestSave( void );
void SV_InitSaveRestore( void );

#define BF_WriteSvc( bf, svc ) BF_WriteByte( bf, svc )
#define BF_ReadSvc( bf ) BF_ReadByte( bf )

#endif 