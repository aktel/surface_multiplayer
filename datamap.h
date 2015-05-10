#ifndef DATAMAP_H
#define DATAMAP_H

#ifdef PRAGMA_ONCE
	#pragma once
#endif

enum DATAMAP_CLASS_TYPE
{
	DTCT_SIZER,
	DTCT_DIRECT,
	DTCT_BACKUP,
	DTCT_INFORMER,
};

#define TEMPLATE_FIELD() template < typename dtT, int defFlag, int defLock >

class Dt_Sizer
{
public:

	TEMPLATE_FIELD()
	class Dt_Foo
	{
	public:
		inline const int GetDefFlag() { return defFlag; }
		inline const int GetDefSize() { return sizeof(Wrk); }
		inline const int GetDefLock() { return defLock; }
		//inline dtT GetAddress() {};
		//inline int GetIndex(); 
		inline void SetAddress( void *address );
	};

	unsigned char a_byte;
};

class Dt_Informer
{
public:
	
	TEMPLATE_FIELD()
	class Dt_Foo
	{
	public:
		inline const int GetDefFlag() { return defFlag; }
		inline const int GetDefSize() { return sizeof(Wrk); }
		inline const int GetDefLock() { return defLock; }

		Dt_Foo( void )
		{
			flags = GetDefFlag();
			lock = GetDefLock();
		}
	

		int flags;
		int loсk;

	};
};

class Dt_Direct
{
public:
	typedef typename Dt_Informer::Dt_Foo BaseFoo;

	TEMPLATE_FIELD()
	class Dt_Foo : public BaseFoo< Wrk, defFlag, defLock >
	{
	public:
		Dt_Foo( void ) {};

		inline void SetAddress( void *address ) { m_pointer = address; }
		inline void SetIndex( int index ) { m_index = index };

		Wrk *m_pointer;
		int m_index;
	};
};

#include <vector>

template < DATAMAP_CLASS_TYPE DTCT > struct DT_REALEASE {};
template <> struct DT_REALEASE<DTCT_SIZER> { typedef Dt_Sizer T };
template <> struct DT_REALEASE<DTCT_DIRECT> { typedef Dt_Direct T };
template <> struct DT_REALEASE<DTCT_BACKUP> { typedef Dt_Backup T };
template <> struct DT_REALEASE<DTCT_INFORMER> { typedef Dt_Informer T };


template < DATAMAP_CLASS_TYPE DTCT > class Dt_HardCommon
{
	typedef typename DT_REALEASE<DTCT>::T T;

	std::vector<T> m_datadesc;
};

template < > class Dt_HardCommon<DTCT_INFORMER>
{
public:
	typedef typename DT_REALEASE<DTCT_INFORMER>::T T;

	static std::vector<T> m_datadesc;
};



#define DEFINE_CLASS_TYPE( className )

#define DECLARE_CLASS_NOBASE( className ) DEFINE_CLASS_TYPE( className ) typedef className thisClass;
#define DECLARE_CLASS_BASE( className, baseClassName ) DECLARE_CLASS_NOBASE( className ) DEFINE_CLASS_TYPE( baseClassName ) typedef baseClassName baseClass;

#define DECLARE_DATAMAP_CLASS( datamap_name ) static DT_Food< ThisClass, datamap_name > m_##datamap_name;
#define NEW_DATAMAP_CLASS( datamap_name ) DECLARE_DATAMAP_CLASS( datamap_name ) BEGIN_DATADESC( className, datamap_name )


#define BEGIN_DATADESC( className, datamap_name ) \
class Dt_Handler_##datamap_name \
{ \
	class ExportHandler \
	{ \
	public: \
		static DataMap *m_datamap; \
		Handler( void ){ m_datamap = thisClass::m_##datamap_name; m_datamap->m_RunTimeName = datamap_name; } \
	}; \
 \
	static Handler m_handler; \
 \
	template < DATAMAP_CLASS_TYPE DTCT > \
	class Dt_HardFoo : public Dt_HardCommon<DTCT> \
	{ \
	public:

#define END_DATADESC() \
}; \
static Dt_HardFoo<DTCT_INFORMER> m_informer; \ 
virtual std::vector<Dt_Informer>* GetInformer() { return &m_informer::m_datadesc; }	\ // Get from dynamic library.
class ExportHandler2	\
{	\
public:	\
	Handler2( void ) \
	{ \
		Handler::m_datamap->m_ProtoData = m_informer.m_datadesc.begin(); \
		Handler::m_datamap->m_DataNumFields = m_informer.m_datadesc.size(); \ 
	} \
}; 


//class Dt_ExportToDynamicLib
//{
//	Dt_HardFoo<DTCT_INFORMER> *m_export_informer; // For export to dynamic library. 
//};
//Dt_ExportToDynamicLib m_export;

#define NEW_DATADESC( datamap_name ) class DataMapGlobal_##datamap_name { public: DECLARE_CLASS_NOBASE( DataMapGlobal_##datamap_name )  };

#define BEGIN_DATADESC_CLASS( datamap_name ) \
	DECLARE_CLASS_NOBASE( thisClass ) \
	BEGIN_DATADESC( ThisClass )

#define DECLARE_FIELD( fieldName, fieldFlag, floatMagnet, fieldLock, initType )

class DataMap
{
public:
	TypeDescription		*m_ProtoData;
	int 				m_DataNumFields;
	const char 			*m_DataClassName;	// thisClass name
	const char 			*m_RunTimeName;	// DT name 
	DataMap 			*m_BaseMap;	// Base
};

typedef DataMap DT_Export;

#endif // DATAMAP_H