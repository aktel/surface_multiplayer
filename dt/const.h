#ifndef CONST_H
#define CONST_H

typedef unsigned int uint;
typedef unsigned long dword;
typedef unsigned char byte;


typedef int		func_t;
typedef int		string_t;

typedef unsigned char	byte;
typedef unsigned short	word;

#undef true
#undef false

#ifndef __cplusplus
typedef enum { false, true }	qboolean;
#else 
typedef int qboolean;
#endif

typedef float vec_t[3];

#endif