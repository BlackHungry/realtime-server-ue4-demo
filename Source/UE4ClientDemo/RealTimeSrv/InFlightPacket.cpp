// Fill out your copyright notice in the Description page of Project Settings.


#include "InFlightPacket.h"
#include "RealTimeSrvTiming.h"



InFlightPacket::InFlightPacket( PacketSN inSequenceNumber ) :
	mSequenceNumber( inSequenceNumber ),
	mTimeDispatched( RealTimeSrvTiming::sInstance->GetCurrentGameTime() )
{
	//null out other transmision data params...
}


void InFlightPacket::HandleDeliveryFailure( DeliveryNotifyMgr* inDeliveryNotificationManager ) const
{
	for ( const auto& pair : mTransmissionDataMap )
	{
		pair.second->HandleDeliveryFailure( inDeliveryNotificationManager );
	}
}

void InFlightPacket::HandleDeliverySuccess( DeliveryNotifyMgr* inDeliveryNotificationManager ) const
{
	for ( const auto& pair : mTransmissionDataMap )
	{
		pair.second->HandleDeliverySuccess( inDeliveryNotificationManager );
	}
}