#ifndef DATAMAP_H
#define DATAMAP_H

#ifdef PRAGMA_ONCE
#pragma once
#endif

#define DECLARE_CLASS_NOBASE( ClassName ) typedef ClassName ThisClass;
#define DECLARE_CLASS_BASE( ClassName, ParentClass ) DECLARE_CLASS_NOBASE( ClassName ) typedef ParentClass BaseClass;

#define DECLARE_DATAMAP_CLASS( Name ) \
static datamap_t *Name##_Datamap;

#ifndef LAMBA_FUNC
	#define TEMPLATE_FIELD() template < typename Fld_T = byte, int flags = 0, typename Adv_Inf = byte >
#else
	// TODO
#endif

#ifdef MSVC
#pragma pack(push, 1)
#endif

TEMPLATE_FIELD()
class Sizer
{
	void SetAddress( void *ptr ){};
	void SetIndex( short index ){ this->index = index; };
	unsigned char a_byte;
};

#ifdef MSVC
#pragma pack(pop)
#endif

TEMPLATE_FIELD()
class DT_Field
{
public:
	void SetAddress( void *ptr ){ address = ptr };
	void SetIndex( short index ){ this->index = index; };
	short index;
	void *address;
};

#include <vector>

class DataMapClass
{
public:

	std::vector<void*> m_fields;
};

#define BEGIN_DATAMAP_CLASS( ClassName, Name ) \
template <typename T = DT_Field>	\
class ClassName##_##_##_##Name : public DataMapClass \
{ \
public: \
	static short dim_count; \
	constexpr ClassName##_##_##_##Name( void ) : dim_count (0) { m_parent = nullptr; }	\
	constexpr ClassName##_##_##_##Name( ClassName *base ) { m_parent = base; }	\
	ClassName *m_parent;	\
	class DataDesc {	public:


#define INCLASS( field ) ( &m_parent->field )

TEMPLATE_FIELD()
class DT_FieldBack : public DT_Field<Fld_T, flags>
{
public:
	Fld_T backup;
};

// for old compilers
#ifndef LAMBA_FUNC

#define DEFINE_FIELD( variableName, variableAddress, variableType, variableFlag ) \
class OldCompilesDTClassFoo_##variableName : public T< variableType, variableFlag > \
{ \
public: \
	constexpr OldCompilesDTClassFoo_##variableName( void ) \
	{ \
		m_fields.push_back( this ); \
		SetAddress( ( variableAddress ) ); \
		SetIndex( ++dim_count ); \
	} \
}; OldCompilesDTClassFoo_##variableName variableName;

#endif

#define END_DATAMAP_CLASS( ClassName, Name ) \
}; DataDesc m_datadesc; }; 
static datamap_t* ClassName::Name = Name##DataMap.Prototype();

#define DT_FIELD_OF_CLASS( mapName, field ) _DT_FIELD_OF_CLASS< ThisClass >( foeld )
#define DT_FIELD_OF_CLASS_BYNAME( mapName, field ) _DT_FIELD_OF_CLASS_BYNAME( mapName, field )

template < typename DT_CLASS, typename DT_NAME, typename FIELD >
constexpr inline int _DT_FIELD_OF_CLASS( )
{
	return DT_CLASS :: DT_NAME.m_datadesc.FIELD.GetIndex();
}

template < typename DT_CLASS, typename DT_NAME, typename FIELD >
constexpr inline int _DT_FIELD_OF_CLASS_BYNAME( )
{
	return 0;
}

#endif // DATAMAP_H