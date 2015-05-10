#ifndef DATAMAP_H
#define DATAMAP_H

#ifdef PRAGMA_ONCE
#pragma once
#endif

#include "const.h"
#include <stddef.h>

#define PRAGMA_PACK_START() #pragma pack (push, 1)
#define PRAGMA_PACK_END() #pragma pack(pop)

#define TEMPLATE_FIELD() template < typename Fld_T = byte, bool inclass = false, dword addressat = 0, uint flags = 0, typename Adv_Inf = byte >

TEMPLATE_FIELD()
class DT_Desc
{
public:
	DT_Desc( void ) : _inclass(inclass){}
	Fld_T *address;
	uint id;
	bool _inclass;
};

TEMPLATE_FIELD()
class DT_Delegator
{
public:
};

#define DECLARE_DATADESC( className ) template <typename T> class CFieldsProto##className;	static  void DataMapInit();	  static CFieldsProto##className<DT_Desc<>> *GetDataDesc();

#pragma pack (push, 1)
TEMPLATE_FIELD()
class Sizer
{ 
public: 
		byte m_noop;
};
#pragma pack(pop)

#define BEGIN_DATADESC_GUTS( className )
template <typename T> 
class CFieldsProto##className 
{  
			
#define DT_FIELD( name, inclass, addressat, type, flags, addvanted ) T<type, inclass, addressat, flags> name;

#define BEGIN_DATADESC_GUTS2( className ) }; CFieldsProto##className<DT_Desc<>> *className::GetDataDesc() { static CFieldsProto##className<DT_Desc<>> dataDesc; return &dataDesc; } void className::DataMapInit() { class CActivateOne { public: CActivateOne() { for ( int i = 0; i<= sizeof(CFieldsProto<Sizer<>>); i++ ) { dword *base = (dword*)GetDataDesc(); (DT_Desc<>*)base[i]->id = i; } } }; static CActivateOne m_activate_once; }

#define DATADESC_END( className ) BEGIN_DATADESC_GUTS2( className ) PRAGMA_PACK_END() 

#endif // DATAMAP_H