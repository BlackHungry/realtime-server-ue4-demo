// Fill out your copyright notice in the Description page of Project Settings.


#include "StateQueue.h"
#include "RealTimeSrvHelper.h"

StateQueue::StateQueue()
{
}

StateQueue::~StateQueue()
{
}

void StateQueue::AddStateData( const FRotator& inRotation, const FVector& inVelocity, const FVector& inLocation, const FRotator& inCameraRotation )
{
	mStateQueue.emplace_back( inRotation, inVelocity, inLocation, inCameraRotation );
	R_LOG_EXTRA();
}

bool StateQueue::GetStateData(StateData& outStateData)
{
	if (!mStateQueue.empty())
	{
		outStateData = mStateQueue.front();
		mStateQueue.pop_front();
		return true;
	}
	return false;
}
