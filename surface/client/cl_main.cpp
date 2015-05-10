#include "common.h"
#include "client.h"

convar_t	*cl_predict;
convar_t	*cl_smooth;
convar_t	*cl_showfps;
convar_t	*cl_envshot_size;
convar_t	*cl_timeout;
convar_t	*cl_nodelta;
convar_t	*cl_interp;
convar_t	*cl_crosshair;
convar_t	*cl_testlights;
convar_t	*cl_solid_players;
convar_t	*cl_idealpitchscale;
convar_t	*cl_allow_levelshots;
convar_t	*cl_lightstyle_lerping;
convar_t	*cl_draw_particles;
convar_t	*cl_levelshot_name;
convar_t	*cl_draw_beams;
convar_t	*scr_centertime;
convar_t	*scr_viewsize;
convar_t	*scr_download;
convar_t	*scr_loading;
convar_t	*scr_dark;	// start from dark
convar_t	*userinfo;

void CL_Init()
{

}

void CL_InitLocal()
{
	// register our variables
	cl_predict = Cvar_Get( "cl_predict", "0", CVAR_ARCHIVE, "disables client movement prediction" );
	cl_crosshair = Cvar_Get( "crosshair", "1", CVAR_ARCHIVE, "show weapon chrosshair" );
	cl_nodelta = Cvar_Get ("cl_nodelta", "0", 0, "disable delta-compression for usercommnds" );
	cl_idealpitchscale = Cvar_Get( "cl_idealpitchscale", "0.8", 0, "how much to look up/down slopes and stairs when not using freelook" );
	cl_solid_players = Cvar_Get( "cl_solid_players", "1", 0, "Make all players not solid (can't traceline them)" );
	cl_interp = Cvar_Get( "ex_interp", "0.1", 0, "Interpolate object positions starting this many seconds in past" ); 
	cl_timeout = Cvar_Get( "cl_timeout", "60", 0, "connect timeout (in-seconds)" );

	rcon_client_password = Cvar_Get( "rcon_password", "", 0, "remote control client password" );
	rcon_address = Cvar_Get( "rcon_address", "", 0, "remote control address" );

	Cvar_Get( "password", "", CVAR_USERINFO, "player password" );
	name = Cvar_Get( "name", Sys_GetCurrentUser(), CVAR_USERINFO|CVAR_ARCHIVE|CVAR_PRINTABLEONLY, "player name" );
	model = Cvar_Get( "model", "player", CVAR_USERINFO|CVAR_ARCHIVE, "player model ('player' it's a single player model)" );
	topcolor = Cvar_Get( "topcolor", "0", CVAR_USERINFO|CVAR_ARCHIVE, "player top color" );
	bottomcolor = Cvar_Get( "bottomcolor", "0", CVAR_USERINFO|CVAR_ARCHIVE, "player bottom color" );
	rate = Cvar_Get( "rate", "25000", CVAR_USERINFO|CVAR_ARCHIVE, "player network rate" );
	hltv = Cvar_Get( "hltv", "0", CVAR_USERINFO|CVAR_LATCH, "HLTV mode" );
	cl_showfps = Cvar_Get( "cl_showfps", "1", CVAR_ARCHIVE, "show client fps" );
	cl_smooth = Cvar_Get ("cl_smooth", "0", CVAR_ARCHIVE, "smooth up stair climbing and interpolate position in multiplayer" );
	cl_cmdbackup = Cvar_Get( "cl_cmdbackup", "10", CVAR_ARCHIVE, "how many additional history commands are sent" );
	cl_cmdrate = Cvar_Get( "cl_cmdrate", "30", CVAR_ARCHIVE, "Max number of command packets sent to server per second" );

	Cmd_AddCommand ("userinfo", CL_Userinfo_f, "print current client userinfo" );
	Cmd_AddCommand ("disconnect", CL_Disconnect_f, "disconnect from server" );
	
	Cmd_AddCommand ("quit", CL_Quit_f, "quit from game" );
	Cmd_AddCommand ("exit", CL_Quit_f, "quit from game" );

	Cmd_AddCommand ("connect", CL_Connect_f, "connect to a server by hostname" );
	Cmd_AddCommand ("reconnect", CL_Reconnect_f, "reconnect to current level" );

	Cmd_AddCommand ("rcon", CL_Rcon_f, "sends a command to the server console (rcon_password and rcon_address required)" );
}