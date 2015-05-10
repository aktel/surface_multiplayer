/*
=====================
Delta_WriteField

write fields by offsets
assume from and to is valid
=====================
*/
qboolean Delta_WriteField( sizebuf_t *msg, delta_t *pField, qboolean write, void *from, void *to, float timebase )
{
	qboolean		bSigned = ( pField->flags & DT_SIGNED ) ? true : false;
	float		flValue, flAngle, flTime;
	uint		iValue;
	const char	*pStr;

	if( Delta_CompareField( pField, from, to, timebase ))
	{
		BF_WriteOneBit( msg, 0 );	// unchanged
		return false;
	}

	BF_WriteOneBit( msg, 1 );	// changed

	if( pField->flags & DT_BYTE )
	{
		iValue = *(byte *)((byte *)to + pField->offset );
		iValue = Delta_ClampIntegerField( iValue, bSigned, pField->bits );
		iValue *= pField->multiplier;
		BF_WriteBitLong( msg, iValue, pField->bits, bSigned );
	}
	else if( pField->flags & DT_SHORT )
	{
		iValue = *(word *)((byte *)to + pField->offset );
		iValue = Delta_ClampIntegerField( iValue, bSigned, pField->bits );
		iValue *= pField->multiplier;
		BF_WriteBitLong( msg, iValue, pField->bits, bSigned );
	}
	else if( pField->flags & DT_INTEGER )
	{
		iValue = *(uint *)((byte *)to + pField->offset );
		iValue = Delta_ClampIntegerField( iValue, bSigned, pField->bits );
		iValue *= pField->multiplier;
		BF_WriteBitLong( msg, iValue, pField->bits, bSigned );
	}
	else if( pField->flags & DT_FLOAT )
	{
		flValue = *(float *)((byte *)to + pField->offset );
		iValue = (int)(flValue * pField->multiplier);
		BF_WriteBitLong( msg, iValue, pField->bits, bSigned );
	}
	else if( pField->flags & DT_ANGLE )
	{
		flAngle = *(float *)((byte *)to + pField->offset );

		// NOTE: never applies multipliers to angle because
		// result may be wrong on client-side
		BF_WriteBitAngle( msg, flAngle, pField->bits );
	}
	else if( pField->flags & DT_TIMEWINDOW_8 )
	{
		flValue = *(float *)((byte *)to + pField->offset );
		flTime = (timebase * 100.0f) - (flValue * 100.0f);
		iValue = (uint)abs( flTime );

		BF_WriteBitLong( msg, iValue, pField->bits, bSigned );
	}
	else if( pField->flags & DT_TIMEWINDOW_BIG )
	{
		flValue = *(float *)((byte *)to + pField->offset );
		flTime = (timebase * pField->multiplier) - (flValue * pField->multiplier);
		iValue = (uint)abs( flTime );

		BF_WriteBitLong( msg, iValue, pField->bits, bSigned );
	}
	else if( pField->flags & DT_STRING )
	{
		pStr = (char *)((byte *)to + pField->offset );

		BF_WriteString( msg, pStr );
	}
	return true;
}

/*
==================
MSG_WriteDeltaEntity

Writes part of a packetentities message, including the entity number.
Can delta from either a baseline or a previous packet_entity
If to is NULL, a remove entity update will be sent
If force is not set, then nothing at all will be generated if the entity is
identical, under the assumption that the in-order delta code will catch it.
==================
*/
void MSG_WriteDeltaEntity( CEntState *from, CEntState *to, sizebuf_t *msg, qboolean force, qboolean player, float timebase ) 
{
	delta_info_t	*dt = NULL;
	delta_t		*pField;
	int		i, startBit;
	int		numChanges = 0;

	if( to == NULL )
	{
		int	fRemoveType;

		if( from == NULL ) return;

		// a NULL to is a delta remove message
		BF_WriteWord( msg, from->number );

		// fRemoveType:
		// 0 - keep alive, has delta-update
		// 1 - remove from delta message (but keep states)
		// 2 - completely remove from server
		if( force ) fRemoveType = 2;
		else fRemoveType = 1;

		BF_WriteUBitLong( msg, fRemoveType, 2 );
		return;
	}

	startBit = msg->iCurBit;

	if( to->number < 0 || to->number >= GI->max_edicts )
		Host_Error( "MSG_WriteDeltaEntity: Bad entity number: %i\n", to->number );

	BF_WriteWord( msg, to->number );
	BF_WriteUBitLong( msg, 0, 2 ); // alive

	if( to->entityType != from->entityType )
	{
		BF_WriteOneBit( msg, 1 );
		BF_WriteUBitLong( msg, to->entityType, 2 );
	}
	else BF_WriteOneBit( msg, 0 ); 

	if( to->entityType == ENTITY_NORMAL )
	{
		if( player )
		{
			dt = Delta_FindStruct( "entity_state_player_t" );
		}
		else
		{
			dt = Delta_FindStruct( "entity_state_t" );
		}
	}
	else if( to->entityType == ENTITY_BEAM )
	{
		dt = Delta_FindStruct( "custom_entity_state_t" );
	}

	ASSERT( dt && dt->bInitialized );
		
	pField = dt->pFields;
	ASSERT( pField );

	// activate fields and call custom encode func
	Delta_CustomEncode( dt, from, to );

	#if 1
	// process fields
	for( i = 0; i < dt->numFields; i++, pField++ )
	{
		if( Delta_WriteField( msg, pField, true, from, to, timebase ))
			numChanges++;
	}

	#else

	#endif

	// if we have no changes - kill the message
	if( !numChanges && !force ) BF_SeekToBit( msg, startBit );
}