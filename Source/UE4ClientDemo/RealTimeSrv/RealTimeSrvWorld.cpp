// Fill out your copyright notice in the Description page of Project Settings.


#include "RealTimeSrvWorld.h"
#include "NetworkMgr.h"
#include "RealTimeSrvHelper.h"




std::unique_ptr< RealTimeSrvWorld >	RealTimeSrvWorld::sInstance;

void RealTimeSrvWorld::StaticInit()
{
	sInstance.reset( new RealTimeSrvWorld() );
}

RealTimeSrvWorld::RealTimeSrvWorld()
{
}


void RealTimeSrvWorld::AddGameObject( RealTimeSrvEntityPtr inGameObject )
{
	mGameObjects.push_back( inGameObject );
	inGameObject->SetIndexInWorld( mGameObjects.size() - 1 );
}

void RealTimeSrvWorld::ResetRealTimeSrvWorld()
{
	for ( int i = 0, c = mGameObjects.size(); i < c; ++i )
	{
		RealTimeSrvEntityPtr go = mGameObjects[i];

		RemoveGameObject( go );
		go->HandleDying();
		--i;
		--c;
	}
}

void RealTimeSrvWorld::RemoveGameObject( RealTimeSrvEntityPtr inGameObject )
{
	int index = inGameObject->GetIndexInWorld();

	int lastIndex = mGameObjects.size() - 1;
	if ( index != lastIndex )
	{
		mGameObjects[index] = mGameObjects[lastIndex];
		mGameObjects[index]->SetIndexInWorld( index );
	}

	inGameObject->SetIndexInWorld( -1 );

	mGameObjects.pop_back();
}


void RealTimeSrvWorld::Update()
{

	for ( int i = 0, c = mGameObjects.size(); i < c; ++i )
	{
		RealTimeSrvEntityPtr go = mGameObjects[i];

		if ( !go->DoesWantToDie() )
		{
			go->Update();
		}

		if ( go->DoesWantToDie() )
		{
			RemoveGameObject( go );
			go->HandleDying();
			--i;
			--c;
		}
	}
}