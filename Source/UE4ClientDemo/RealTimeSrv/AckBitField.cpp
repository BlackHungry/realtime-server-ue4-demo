#include "AckBitField.h"


void AckBitField::AddLastBit( uint32_t inTotalDifference )
{
	uint32_t byteOffset = ( inTotalDifference - 1 ) >> 3;
	uint32_t bitOffset = ( inTotalDifference - 1 ) & 0x7;

	uint8_t tempMask = 0x01 << bitOffset;

	*( mAckBitField + byteOffset ) |= tempMask;
}

void AckBitField::DoAddToAckBitField( uint32_t inDifference )
{
	uint8_t temp_uint8 = 0;
	for ( int i = ACK_BIT_FIELD_BYTE_LEN - 1; i > 0; --i )
	{
		*( mAckBitField + i ) = ( *( mAckBitField + i ) << inDifference );

		if ( i - 1 >= 0 )
		{
			temp_uint8 = *( mAckBitField + i - 1 );

			*( mAckBitField + i ) |=
				( temp_uint8 >> ( 8 - inDifference ) );
		}
	}

	( *mAckBitField ) = ( *mAckBitField ) << inDifference;

	//cout << "*******************\n";
	//uint8_t tt = *( mAckBitField + 0 );
	//uint8_t tt1 = *( mAckBitField + 1 );
	//uint8_t tt2 = *( mAckBitField + 2 );
	//uint8_t tt3 = *( mAckBitField + 3 );

	//printf( "tt = %d \n", tt );
	//printf( "tt1 = %d \n", tt1 );
	//printf( "tt2 = %d \n", tt2 );
	//printf( "tt3 = %d \n", tt3 );
	//cout << "*******************\n";
}

void AckBitField::AddToAckBitField( PacketSN inSequenceNumber, PacketSN inLastSN )
{
	//cout << "inSequenceNumber = " << inSequenceNumber << endl;
	//cout << "inLastSN = " << inLastSN << endl;

	mLatestAckSN = inSequenceNumber;

	static bool isFirstTime = true;
	if ( isFirstTime )
	{
		isFirstTime = false;
		return;
	}

	uint32_t totalDifference = 0;
	for ( ; RealTimeSrvHelper::SequenceGreaterThan( inSequenceNumber, inLastSN++ ); ++totalDifference );
	//for ( ; inSequenceNumber > inLastSN++ ; ++totalDifference );

	//cout << "totalDifference = " << totalDifference << endl;

	uint32_t tempDiff = totalDifference;
	while ( tempDiff > 8 )
	{
		DoAddToAckBitField( 8 );
		tempDiff -= 8;
	}
	if ( tempDiff > 0 )
	{
		//cout << "DoAddToAckBitField( tempDiff ) : tempDiff = " << tempDiff << endl;
		DoAddToAckBitField( tempDiff );
	}
	AddLastBit( totalDifference );
}

void AckBitField::Write( OutputBitStream& inOutputStream )
{
	inOutputStream.Write( mLatestAckSN );
	inOutputStream.WriteBytes( mAckBitField, ACK_BIT_FIELD_BYTE_LEN );
}

void AckBitField::Read( InputBitStream& inInputStream )
{
	inInputStream.Read( mLatestAckSN );
	inInputStream.ReadBytes( mAckBitField, ACK_BIT_FIELD_BYTE_LEN );
}
