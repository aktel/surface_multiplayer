#ifndef ROOT_COMMON_H
#define ROOT_COMMON_H

#ifdef PRAGMA_ONCE
#pragma once
#endif

#define MAX_STRING		256	// generic string
#define MAX_INFO_STRING	256	// infostrings are transmitted across network
#define MAX_SYSPATH		1024	// system filepath
#define MAX_MODS		512	// environment games that engine can keep visible
#define BIT( n )		(1<<( n ))

#ifndef __cplusplus
#define NULL		((void *)0)
#endif

#define PROG_VERSION	0.1f		// engine current version

// PERFORMANCE INFO
#define MIN_FPS         	15.0		// host minimum fps value for maxfps.
#define MAX_FPS         	500.0		// upper limit for maxfps.

#define MAX_FRAMETIME	0.1
#define MIN_FRAMETIME	0.000001

#define MAX_CMD_TOKENS	80		// cmd tokens
#define MAX_ENTNUMBER	99999		// for server and client parsing
#define MAX_HEARTBEAT	-99999		// connection time
#define QCHAR_WIDTH		16		// font width

#define CIN_MAIN		0
#define CIN_LOGO		1

#define MAX_NUM_ARGVS	128

// filesystem flags
#define FS_STATIC_PATH	1	// FS_ClearSearchPath will be ignore this path
#define FS_NOWRITE_PATH	2	// default behavior - last added gamedir set as writedir. This flag disables it
#define FS_GAMEDIR_PATH	4	// just a marker for gamedir path

// config strings are a general means of communication from
// the server to all connected clients.
// each config string can be at most CS_SIZE characters.
#define CS_SIZE		64	// size of one config string
#define CS_TIME		16	// size of time string

#ifdef MSVC
	#pragma warning(disable : 4244)	// MIPS
	#pragma warning(disable : 4018)	// signed/unsigned mismatch
	#pragma warning(disable : 4305)	// truncation from const double to float
	#pragma warning(disable : 4115)	// named type definition in parentheses
	#pragma warning(disable : 4100)	// unreferenced formal parameter
	#pragma warning(disable : 4127)	// conditional expression is constant
	#pragma warning(disable : 4057)	// differs in indirection to slightly different base types
	#pragma warning(disable : 4201)	// nonstandard extension used
	#pragma warning(disable : 4706)	// assignment within conditional expression
	#pragma warning(disable : 4054)	// type cast' : from function pointer
	#pragma warning(disable : 4310)	// cast truncates constant value
#endif

typedef unsigned long	dword;
typedef unsigned int	uint;
typedef char		string[MAX_STRING];
typedef long		fs_offset_t;
typedef struct file_s	file_t;		// normal file
typedef struct stream_s	stream_t;		// sound stream for background music playing

#include "const.h"
#include "system.h"
#include "crtlib.h"

typedef struct
{
	int	numfilenames;
	char	**filenames;
	char	*filenamesbuffer;
} search_t;

enum
{
	D_INFO = 1,	// "-dev 1", shows various system messages
	D_WARN,		// "-dev 2", shows not critical system warnings
	D_ERROR,		// "-dev 3", shows critical warnings 
	D_AICONSOLE,	// "-dev 4", special case for game aiconsole
	D_NOTE		// "-dev 5", show system notifications for engine developers
};

typedef enum
{	
	HOST_NORMAL,	// listen server, singleplayer
	HOST_DEDICATED,
} instance_t;

#endif // ROOT_COMMON_H