// Fill out your copyright notice in the Description page of Project Settings.


#include "RealTimeSrvPawn.h"
#include "InputMgr.h"
#include "RealTimeSrvHelper.h"
#include "RealTimeSrvTiming.h"
#include "NetworkMgr.h"
#include "kismet/GameplayStatics.h"



uint32_t ARealTimeSrvPawn::GetClassId() const
{
	return 'CHRT';
}

ARealTimeSrvPawn::ARealTimeSrvPawn( const FObjectInitializer& ObjectInitializer )
	: Super( ObjectInitializer )
{

	RootComponent = ObjectInitializer.CreateDefaultSubobject<USceneComponent>( this, TEXT( "RootComponent" ) );

	RealTimeSrvPawnCamera = ObjectInitializer.CreateDefaultSubobject<UCameraComponent>( this, TEXT( "RealTimeSrvPawnCamera" ) );
	RealTimeSrvPawnCamera->SetupAttachment( RootComponent );

	FTransform CameraTransform( FRotator::ZeroRotator, FVector( 0.f, 0.f, 70.f ) );
	RealTimeSrvPawnCamera->SetRelativeTransform( CameraTransform );

	Mesh1P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>( this, TEXT( "PawnMesh1P" ) );
	Mesh1P->SetupAttachment( RealTimeSrvPawnCamera );
	Mesh1P->bOnlyOwnerSee = true;
	Mesh1P->bOwnerNoSee = false;
	Mesh1P->bCastDynamicShadow = false;
	Mesh1P->bReceivesDecals = false;
	Mesh1P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh1P->PrimaryComponentTick.TickGroup = TG_PrePhysics;

	FTransform Mesh1PTransform( FRotator( 0.f, -90.f, 0.f ), FVector( 0.f, 0.f, -150.f ) );
	Mesh1P->SetRelativeTransform( Mesh1PTransform );

	Mesh3P = ObjectInitializer.CreateDefaultSubobject<USkeletalMeshComponent>( this, TEXT( "PawnMesh3P" ) );
	Mesh3P->SetupAttachment( RootComponent );
	Mesh3P->bOnlyOwnerSee = false;
	Mesh3P->bOwnerNoSee = true;
	Mesh3P->bReceivesDecals = false;
	Mesh3P->MeshComponentUpdateFlag = EMeshComponentUpdateFlag::OnlyTickPoseWhenRendered;
	Mesh3P->PrimaryComponentTick.TickGroup = TG_PrePhysics;

	FTransform Mesh3PTransform( FRotator( 0.f, -90.f, 0.f ), FVector( 0.f, 0.f, -86.f ) );
	Mesh3P->SetRelativeTransform( Mesh3PTransform );

	PrimaryActorTick.bCanEverTick = true;

	mPlayerId = 0;
	mPlayerIDAlreadySetFlag = false;

	BaseTurnRate = 2.f;
	BaseLookUpRate = 2.f;

	MaxSpeed = 440.f;
	Acceleration = 1000.f;
	Deceleration = 2000.f;
	TurningBoost = 8.0f;

	mVelocity = FVector::ZeroVector;

	mCameraRotation = FRotator::ZeroRotator;
	mLocalCameraRotation = FRotator::ZeroRotator;

	mLocalRotation = FRotator::ZeroRotator;

	mLocalLocation = FVector::ZeroVector;

	bIsPlayerLocationOutOfSync = false;

	mLocalVelocity = FVector::ZeroVector;

	bTestUpdateForDisconnect = false;

	mRemotePawnTargetLocation = FVector::ZeroVector;
	mRemotePawnTargetVelocity = FVector::ZeroVector;
	mRemotePawnTargetRotation = FRotator::ZeroRotator;
	mRemotePawnTargetCameraRotation = FRotator::ZeroRotator;

	mTimeOfLastUpdateTargetState = 0.f;
}

void ARealTimeSrvPawn::BeginPlay()
{
	Super::BeginPlay();

	mCameraRotation = GetActorRotation();
	mLocalCameraRotation = mCameraRotation;
}

void ARealTimeSrvPawn::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	if ( bTestUpdateForDisconnect )
	{
		Update();
	}
}

void ARealTimeSrvPawn::SetupPlayerInputComponent( UInputComponent* PlayerInputComponent )
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	check( PlayerInputComponent );
	PlayerInputComponent->BindAxis( "MoveForward", this, &ARealTimeSrvPawn::MoveForward );
	PlayerInputComponent->BindAxis( "MoveRight", this, &ARealTimeSrvPawn::MoveRight );
	PlayerInputComponent->BindAxis( "Turn", this, &ARealTimeSrvPawn::Turn );
	PlayerInputComponent->BindAxis( "LookUp", this, &ARealTimeSrvPawn::LookUp );
}

void ARealTimeSrvPawn::MoveForward( float Val )
{
	//if ( Val != 0.f )
	//{
	//const FQuat Rotation = GetActorQuat();
	//const FVector Direction = FQuatRotationMatrix( Rotation ).GetScaledAxis( EAxis::X );
	//ActionAddMovementInput( Direction * Val );

	InputMgr::sInstance->HandleMoveInput( InputMgr::EIA_MoveForward, Val );
	//}
}

void ARealTimeSrvPawn::MoveRight( float Val )
{
	//if (Val != 0.f)
	//{
	//const FQuat Rotation = GetActorQuat();
	//const FVector Direction = FQuatRotationMatrix( Rotation ).GetScaledAxis( EAxis::Y );
	//ActionAddMovementInput( Direction * Val );

	InputMgr::sInstance->HandleMoveInput( InputMgr::EIA_MoveRight, Val );
	//}
}

void ARealTimeSrvPawn::Turn( float Val )
{
	mLocalRotation = GetActorRotation();
	mLocalRotation.Yaw += ( BaseTurnRate * Val );

	InputMgr::sInstance->HandleTurnInput(
		InputMgr::EIA_Turn,
		mLocalRotation.Pitch,
		mLocalRotation.Yaw,
		mLocalRotation.Roll
	);


	//APawn::AddControllerYawInput( Val );
	//mLocalRotation.Yaw = GetControlRotation().Yaw;

	//InputMgr::sInstance->HandleTurnInput(
	//	InputMgr::EIA_Turn,
	//	mLocalRotation.Pitch,
	//	mLocalRotation.Yaw,
	//	mLocalRotation.Roll
	//);
}

void ARealTimeSrvPawn::LookUp( float Val )
{
	if ( RealTimeSrvPawnCamera )
	{
		mLocalCameraRotation.Yaw = GetActorRotation().Yaw;
		mLocalCameraRotation.Roll = GetActorRotation().Roll;

		mLocalCameraRotation.Pitch = FMath::Clamp( ( mLocalCameraRotation.Pitch + ( -1 * BaseLookUpRate * Val ) ), -89.f, 89.f );

		InputMgr::sInstance->HandleTurnInput(
			InputMgr::EIA_LookUp,
			mLocalCameraRotation.Pitch,
			mLocalCameraRotation.Yaw,
			mLocalCameraRotation.Roll
		);

		//R_LOG_N("Val = ", Val);
	}


	//APawn::AddControllerPitchInput( Val );
	//if ( RealTimeSrvPawnCamera )
	//{
	//	//mLocalCameraRotation.Yaw = GetActorRotation().Yaw;
	//	//mLocalCameraRotation.Roll = GetActorRotation().Roll;
	//	//mLocalCameraRotation.Pitch = FMath::Clamp( ( mLocalCameraRotation.Pitch + ( -1 * BaseLookUpRate * Val ) ), -89.f, 89.f );

	//	mLocalCameraRotation = GetControlRotation();
	//	InputMgr::sInstance->HandleTurnInput(
	//		InputMgr::EIA_LookUp,
	//		mLocalCameraRotation.Pitch,
	//		mLocalCameraRotation.Yaw,
	//		mLocalCameraRotation.Roll
	//	);

	//	//R_LOG_N("Val = ", Val);
	//}
}

void ARealTimeSrvPawn::Update()
{
	if ( GetPlayerId() == NetworkMgr::sInstance->GetPlayerId() || bTestUpdateForDisconnect )
	{
		const Action* pendingMove = InputMgr::sInstance->GetAndClearPendingAction();

		if ( pendingMove )
		{
			float deltaTime = pendingMove->GetDeltaTime();

			ProcessInputBaseOnLocalState( deltaTime, pendingMove->GetInputState() );
			SimulateMovementForLocalPawn( deltaTime );
		}
	}
	else
	{
		SimulateMovementForRemotePawn( RealTimeSrvTiming::sInstance->GetDeltaTime() );
	}
}

void ARealTimeSrvPawn::ProcessInputBaseOnLocalState( float inDeltaTime, const RealTimeSrvInputState& inInputState )
{

	ActionAddMovementInput( mLocalCameraRotation.Quaternion().GetForwardVector(), inInputState.GetDesiredMoveForwardAmount() );
	ActionAddMovementInput( mLocalCameraRotation.Quaternion().GetRightVector(), inInputState.GetDesiredMoveRightAmount() );

	//ApplyControlInputToVelocity( inDeltaTime );
	ApplyControlInputToVelocity( inDeltaTime, mLocalVelocity );

	//SetVelocity();

	FVector Delta = mLocalVelocity * inDeltaTime;

	if ( !Delta.IsNearlyZero( 1e-6f ) )
	{
		//SetActorLocation( GetActorLocation() + Delta );
		mLocalLocation = GetActorLocation() + Delta;
	}

	// AddMovementInput( mLocalActionPawnCameraRotation.Quaternion().GetForwardVector(), inInputState.GetDesiredMoveForwardAmount() );
	// AddMovementInput( mLocalActionPawnCameraRotation.Quaternion().GetRightVector(), inInputState.GetDesiredMoveRightAmount() );
}

void ARealTimeSrvPawn::ProcessInputBaseOnServerState( float inDeltaTime, const RealTimeSrvInputState& inInputState )
{
	//FRotator newRot( GetActorRotation() );
	//mRotation.Yaw += ( BaseTurnRate * inInputState.GetDesiredTurnAmount() );
	mRotation = inInputState.GetDesiredTurnRot();

	//mActionPawnCameraRotation.Yaw = mRotation.Yaw;
	//mActionPawnCameraRotation.Roll = mRotation.Roll;
	//mActionPawnCameraRotation.Pitch = FMath::Clamp( ( mActionPawnCameraRotation.Pitch + ( -1 * BaseLookUpRate * inInputState.GetDesiredLookUpAmount() ) ), -89.f, 89.f );
	//ActionPawnCamera->SetWorldRotation( ActionPawnCameraRotation );
	mCameraRotation = inInputState.GetDesiredLookUpRot();

	ActionAddMovementInput( mCameraRotation.Quaternion().GetForwardVector(), inInputState.GetDesiredMoveForwardAmount() );
	ActionAddMovementInput( mCameraRotation.Quaternion().GetRightVector(), inInputState.GetDesiredMoveRightAmount() );

	//ApplyControlInputToVelocity( inDeltaTime );
	ApplyControlInputToVelocity( inDeltaTime, mVelocity );

	FVector Delta = mVelocity * inDeltaTime;

	if ( !Delta.IsNearlyZero( 1e-6f ) )
	{
		mLocation += Delta;
	}
}

void ARealTimeSrvPawn::UpdateTargetState()
{
	StateQueue::StateData stateData;
	if ( !mStateBuffer.GetStateData( stateData ) )
	{
		return;
		R_LOG_1_EXTRA("UpdateTargetState_return_with no state data");
	}

	mRemotePawnTargetLocation = stateData.GetLocation();
	mRemotePawnTargetVelocity = stateData.GetVelocity();
	mRemotePawnTargetRotation = stateData.GetRotation();
	mRemotePawnTargetCameraRotation = stateData.GetCameraRotation();


	//mInterpSpeedToRemotePawnTargetLocation = 
	//	( mRemotePawnTargetLocation - GetActorLocation() ).Size() 
	//	/ 
	//	NetworkMgr::kTimeBetweenStatePackets;

	//mInterpSpeedToRemotePawnTargetVelocity = 
	//	( mRemotePawnTargetVelocity - GetLocalVelocity() ).Size()
	//	/ 
	//	NetworkMgr::kTimeBetweenStatePackets;

	//mInterpSpeedToRemotePawnTargetRotation = 
	//	( mRemotePawnTargetRotation - GetActorRotation() ). 
	//	/ 
	//	NetworkManager::kTimeBetweenStatePackets;

	//mInterpSpeedToRemotePawnTargetCameraRotation = 
	//	( mRemotePawnTargetCameraRotation - ActionPawnCamera->GetComponentRotation() ).Size() 
	//	/ 
	//	NetworkManager::kTimeBetweenStatePackets;
	R_LOG_M_EXTRA( "READ!!! mRemotePawnTargetLocation = %f, %f, %f", mRemotePawnTargetLocation.X, mRemotePawnTargetLocation.Y, mRemotePawnTargetLocation.Z );
	//R_LOG_M_EXTRA( "READ!!! mInterpSpeedToRemotePawnTargetLocation = %f", mInterpSpeedToRemotePawnTargetLocation );
	R_LOG_1_EXTRA( "------------------------" );

	R_LOG_M_EXTRA( "READ!!! mRemotePawnTargetVelocity = %f, %f, %f", mRemotePawnTargetVelocity.X, mRemotePawnTargetVelocity.Y, mRemotePawnTargetVelocity.Z );
	//R_LOG_M_EXTRA( "READ!!! mInterpSpeedToRemotePawnTargetVelocity = %f", mInterpSpeedToRemotePawnTargetVelocity );
	R_LOG_1_EXTRA( "------------------------" );

	R_LOG_M_EXTRA( "READ!!! mRemotePawnTargetRotation = %f, %f, %f", mRemotePawnTargetRotation.Pitch, mRemotePawnTargetRotation.Yaw, mRemotePawnTargetRotation.Roll );
	R_LOG_1_EXTRA( "------------------------" );

	R_LOG_M_EXTRA( "READ!!! mRemotePawnTargetCameraRotation = %f, %f, %f", mRemotePawnTargetCameraRotation.Pitch, mRemotePawnTargetCameraRotation.Yaw, mRemotePawnTargetCameraRotation.Roll );


}

void ARealTimeSrvPawn::SimulateMovementForRemotePawn( float inDeltaTime )
{
	//{
	//	R_LOG_1_EXTRA( "~~%%%%%%%%%%%%%%%%%%%SimulateMovementForRemotePawn%%%%%%Before%%%%%%%%%%%%%~~~~" )
	//		R_LOG_M_EXTRA( "READ!!! GetActorLocation() = %f, %f, %f", GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z );
	//	R_LOG_M_EXTRA( "READ!!! GetLocation() = %f, %f, %f", GetLocation().X, GetLocation().Y, GetLocation().Z );
	//	R_LOG_M_EXTRA( "READ!!! mRemotePawnTargetLocation = %f, %f, %f", mRemotePawnTargetLocation.X, mRemotePawnTargetLocation.Y, mRemotePawnTargetLocation.Z );
	//	R_LOG_1_EXTRA( "------------------------" );
	//	R_LOG_M_EXTRA( "READ!!! mLocalVelocity = %f, %f, %f", mLocalVelocity.X, mLocalVelocity.Y, mLocalVelocity.Z );
	//	R_LOG_M_EXTRA( "READ!!! mVelocity = %f, %f, %f", mVelocity.X, mVelocity.Y, mVelocity.Z );
	//	R_LOG_M_EXTRA( "READ!!! mRemotePawnTargetVelocity = %f, %f, %f", mRemotePawnTargetVelocity.X, mRemotePawnTargetVelocity.Y, mRemotePawnTargetVelocity.Z );
	//	R_LOG_1_EXTRA( "------------------------" );
	//	R_LOG_M_EXTRA( "READ!!! GetActorRotation() = %f, %f, %f", GetActorRotation().Pitch, GetActorRotation().Yaw, GetActorRotation().Roll );
	//	R_LOG_M_EXTRA( "READ!!! GetRotation() = %f, %f, %f", GetRotation().Pitch, GetRotation().Yaw, GetRotation().Roll );
	//	R_LOG_M_EXTRA( "READ!!! mRemotePawnTargetRotation = %f, %f, %f", mRemotePawnTargetRotation.Pitch, mRemotePawnTargetRotation.Yaw, mRemotePawnTargetRotation.Roll );
	//	R_LOG_1_EXTRA( "------------------------" );
	//	R_LOG_M_EXTRA( "READ!!! ActionPawnCamera->GetComponentRotation() = %f, %f, %f", RealTimeSrvPawnCamera->GetComponentRotation().Pitch, RealTimeSrvPawnCamera->GetComponentRotation().Yaw, RealTimeSrvPawnCamera->GetComponentRotation().Roll );
	//	R_LOG_M_EXTRA( "READ!!! mActionPawnCameraRotation = %f, %f, %f", mCameraRotation.Pitch, mCameraRotation.Yaw, mCameraRotation.Roll );
	//	R_LOG_M_EXTRA( "READ!!! mRemotePawnTargetCameraRotation = %f, %f, %f", mRemotePawnTargetCameraRotation.Pitch, mRemotePawnTargetCameraRotation.Yaw, mRemotePawnTargetCameraRotation.Roll );
	//	R_LOG_1_EXTRA( "%%%%%%%%%%%%%%%%%%%~~~SimulateMovementForRemotePawn%%%%%%%Before%%%%%%%%%%%%~~~" );
	//}

	//R_LOG_N_EXTRA( "ActionTiming::sInstance.GetCurrentGameTime() = ", ActionTiming::sInstance.GetCurrentGameTime() );

	float currentGameTime = RealTimeSrvTiming::sInstance->GetCurrentGameTime();

	if ( currentGameTime < mIsTimeToStartSimulateMovementForRemotePawn )
	{
		//R_LOG_1_EXTRA( "RTSTimming::sInstance.GetCurrentGameTime() < mIsTimeToStartSimulateMovementForRemotePawn" );
		return;
	}


	if ( currentGameTime > mTimeOfLastUpdateTargetState + NetworkMgr::kTimeBetweenStatePackets )
	{
		mTimeOfLastUpdateTargetState = currentGameTime;
		UpdateTargetState();
	}

	//mLocalLocation = mRemotePawnTargetLocation;
	//mLocalVelocity = mRemotePawnTargetVelocity;

	//R_LOG_N_EXTRA( "VInterpTo_Constant start ActionTiming::sInstance.GetCurrentGameTime() = ", ActionTiming::sInstance.GetCurrentGameTime() );
	// mLocalLocation = UKismetMathLibrary::VInterpTo_Constant(
	mLocalLocation = FMath::VInterpConstantTo(
		GetActorLocation(),
		mRemotePawnTargetLocation,
		inDeltaTime,
		mRemotePawnTargetVelocity.Size()
	);
	//R_LOG_N_EXTRA( "VInterpTo_Constant end ActionTiming::sInstance.GetCurrentGameTime() = ", ActionTiming::sInstance.GetCurrentGameTime() );


	// mLocalVelocity = UKismetMathLibrary::VInterpTo_Constant(
	mLocalVelocity = FMath::VInterpConstantTo(
		GetLocalVelocity(),
		mRemotePawnTargetVelocity,
		inDeltaTime,
		Acceleration * 2.f
	);

	//FVector Delta = mLocalVelocity * inDeltaTime;
	//if ( !Delta.IsNearlyZero( 1e-6f ) )
	//{
	//	//mLocalLocation = ( GetActorLocation() + Delta );
	//	mLocalLocation = ( mRemotePawnTargetLocation + Delta );
	//}


	// mLocalRotation = UKismetMathLibrary::RInterpTo(
	mLocalRotation = FMath::RInterpTo(
		GetActorRotation(),
		mRemotePawnTargetRotation,
		inDeltaTime,
		BaseTurnRate * 8.f
	);
	

	//mLocalActionPawnCameraRotation = UKismetMathLibrary::RInterpTo(
	mLocalCameraRotation = FMath::RInterpTo(
		RealTimeSrvPawnCamera->GetComponentRotation(),
		mRemotePawnTargetCameraRotation,
		inDeltaTime,
		BaseLookUpRate * 8.f
	);

	//{
	//	R_LOG_1_EXTRA( "~~***************SimulateMovementForRemotePawn*%After***************%~~~~" );
	//	R_LOG_M_EXTRA( "READ!!! mLocalLocation = %f, %f, %f", mLocalLocation.X, mLocalLocation.Y, mLocalLocation.Z );
	//	R_LOG_1_EXTRA( "------------------------" );
	//	R_LOG_M_EXTRA( "READ!!! mLocalVelocity = %f, %f, %f", mLocalVelocity.X, mLocalVelocity.Y, mLocalVelocity.Z );
	//	R_LOG_1_EXTRA( "------------------------" );
	//	R_LOG_M_EXTRA( "READ!!! mLocalRotation = %f, %f, %f", mLocalRotation.Pitch, mLocalRotation.Yaw, mLocalRotation.Roll );
	//	R_LOG_1_EXTRA( "------------------------" );
	//	R_LOG_M_EXTRA( "READ!!! mLocalActionPawnCameraRotation = %f, %f, %f", mLocalCameraRotation.Pitch, mLocalCameraRotation.Yaw, mLocalCameraRotation.Roll );
	//	R_LOG_1_EXTRA( "------------------------" );
	//	R_LOG_1_EXTRA( "***************~~~SimulateMovementForRemotePawn**************%After**************%%%%~~~" );
	//}

	SetActorLocation( mLocalLocation );
	SetLocalVelocity( mLocalVelocity );
	SetActorRotation( mLocalRotation );
	RealTimeSrvPawnCamera->SetWorldRotation( mLocalCameraRotation );
}

void ARealTimeSrvPawn::SimulateMovementForLocalPawn(float inDeltaTime)
{
	//R_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );

	//R_LOG_M( "READ!!! GetActorRotation() = %f, %f, %f", GetActorRotation().Pitch, GetActorRotation().Yaw, GetActorRotation().Roll );
	//R_LOG_M( "READ!!! mLocalRotation = %f, %f, %f", mLocalRotation.Pitch, mLocalRotation.Yaw, mLocalRotation.Roll );
	//R_LOG_M( "READ!!! GetRotation() = %f, %f, %f", GetRotation().Pitch, GetRotation().Yaw, GetRotation().Roll );
	//R_LOG_1( "------------------------" );

	//R_LOG_M( "READ!!! mLocalActionPawnCameraRotation = %f, %f, %f", mLocalActionPawnCameraRotation.Pitch, mLocalActionPawnCameraRotation.Yaw, mLocalActionPawnCameraRotation.Roll );
	//R_LOG_M( "READ!!! ActionPawnCamera->GetComponentRotation() = %f, %f, %f", ActionPawnCamera->GetComponentRotation().Pitch, ActionPawnCamera->GetComponentRotation().Yaw, ActionPawnCamera->GetComponentRotation().Roll );

	//R_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );

	SetActorRotation( mLocalRotation );
	RealTimeSrvPawnCamera->SetWorldRotation( mLocalCameraRotation );

	if ( bIsPlayerLocationOutOfSync )
	{
		{
			R_LOG_1( "serverrrrrrrrrrrrrrrrrrr before" );

			R_LOG_M( "GetActorLocation() = %f, %f, %f", GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z );
			R_LOG_M( "mLocalLocation = %f, %f, %f", mLocalLocation.X, mLocalLocation.Y, mLocalLocation.Z );
			R_LOG_M( "GetLocation() = %f, %f, %f", GetLocation().X, GetLocation().Y, GetLocation().Z );
			R_LOG_1( "------------------------" );

			R_LOG_M( "mVelocity = %f, %f, %f", mVelocity.X, mVelocity.Y, mVelocity.Z );
			R_LOG_M( "mLocalVelocity = %f, %f, %f", mLocalVelocity.X, mLocalVelocity.Y, mLocalVelocity.Z );

			R_LOG_1( "serverrrrrrrrrrrrrrrrrrr before" );
		}

		bIsPlayerLocationOutOfSync = false;
		if ( !GetActorLocation().Equals( mLocation, 100.f ) )
		{
			mLocalLocation = mLocation;
			mLocalVelocity = mVelocity;
			// SetActorLocation( mLocation );
			
			R_MSG_1( 5.f, "drawwwwwwwwwwwwwwwww" );
			R_LOG_1("drawwwwwwwwwwwwwwwww" );

			GEngine->AddOnScreenDebugMessage( -1, 3.f, FColor::Red,
				FString::Printf( TEXT( "%s" ),
					*FString( "drawwwwwwwwwwwwwwwww" ) )
			);
		}
		else
		{
			//mLocalLocation = FMath::VInterpConstantTo(
			//	GetActorLocation(),
			//	mLocation,
			//	inDeltaTime,
			//	mVelocity.Size() * 0.1f
			//);

			//mLocalVelocity = FMath::VInterpConstantTo(
			//	GetLocalVelocity(),
			//	mVelocity,
			//	inDeltaTime,
			//	Acceleration * 0.1f
			//);
		}

		{
			R_LOG_1( "serverrrrrrrrrrrrrrrrrrr after" );
			R_LOG_M( "mLocalLocation = %f, %f, %f", mLocalLocation.X, mLocalLocation.Y, mLocalLocation.Z );
			R_LOG_1( "------------------------" );
			R_LOG_M( "mLocalVelocity = %f, %f, %f", mLocalVelocity.X, mLocalVelocity.Y, mLocalVelocity.Z );
			R_LOG_1( "serverrrrrrrrrrrrrrrrrrr after" );
		}
	}
	else
	{
		//R_LOG_1( "localllllllllllll" );
		//SetActorLocation( mLocalLocation );
		//R_LOG_M( "GetActorLocation() = %f, %f, %f", GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z );
	}

	SetActorLocation( mLocalLocation );
	//SetLocalVelocity( mLocalVelocity );
}

void ARealTimeSrvPawn::DR( float inDeltaTime )
{
	FVector Delta = mLocalVelocity * inDeltaTime;
	if ( !Delta.IsNearlyZero( 1e-6f ) )
	{
		SetLocation( GetActorLocation() + Delta );
	}
}

bool ARealTimeSrvPawn::IsExceedingMaxSpeed( float inMaxSpeed ) const
{
	inMaxSpeed = FMath::Max( 0.f, inMaxSpeed );
	const float MaxSpeedSquared = FMath::Square( inMaxSpeed );

	// Allow 1% error tolerance, to account for numeric imprecision.
	const float OverVelocityPercent = 1.01f;
	return ( mVelocity.SizeSquared() > MaxSpeedSquared * OverVelocityPercent );
}

void ARealTimeSrvPawn::ActionAddMovementInput( FVector WorldDirection, float ScaleValue /*= 1.0f*/ )
{
	ActionControlInputVector += WorldDirection * ScaleValue;
}

FVector ARealTimeSrvPawn::ActionConsumeMovementInputVector()
{
	ActionLastControlInputVector = ActionControlInputVector;
	ActionControlInputVector = FVector::ZeroVector;
	return ActionLastControlInputVector;
}

FVector ARealTimeSrvPawn::ActionGetPendingInputVector() const
{
	// There's really no point redirecting to the MovementComponent since GetInputVector is not virtual there, and it just comes back to us.
	return ActionControlInputVector;
}

//void ARealTimeSrvPawn::ApplyControlInputToVelocity( float DeltaTime )
//{
//	const FVector ControlAcceleration = ActionGetPendingInputVector().GetClampedToMaxSize( 1.f );
//
//	const float AnalogInputModifier = ( ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f );
//	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;
//	const bool bExceedingMaxSpeed = IsExceedingMaxSpeed( MaxPawnSpeed );
//
//	if ( AnalogInputModifier > 0.f && !bExceedingMaxSpeed )
//	{
//		// Apply change in velocity direction
//		if ( mVelocity.SizeSquared() > 0.f )
//		{
//			// Change direction faster than only using acceleration, but never increase velocity magnitude.
//			const float TimeScale = FMath::Clamp( DeltaTime * TurningBoost, 0.f, 1.f );
//			mVelocity = mVelocity + ( ControlAcceleration * mVelocity.Size() - mVelocity ) * TimeScale;
//		}
//	}
//	else
//	{
//		// Dampen velocity magnitude based on deceleration.
//		if ( mVelocity.SizeSquared() > 0.f )
//		{
//			const FVector OldVelocity = mVelocity;
//			const float VelSize = FMath::Max( mVelocity.Size() - FMath::Abs( Deceleration ) * DeltaTime, 0.f );
//			mVelocity = mVelocity.GetSafeNormal() * VelSize;
//
//			// Don't allow braking to lower us below max speed if we started above it.
//			if ( bExceedingMaxSpeed && mVelocity.SizeSquared() < FMath::Square( MaxPawnSpeed ) )
//			{
//				mVelocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
//			}
//		}
//	}
//
//	// Apply acceleration and clamp velocity magnitude.
//	const float NewMaxSpeed = ( IsExceedingMaxSpeed( MaxPawnSpeed ) ) ? mVelocity.Size() : MaxPawnSpeed;
//	mVelocity += ControlAcceleration * FMath::Abs( Acceleration ) * DeltaTime;
//	mVelocity = mVelocity.GetClampedToMaxSize( NewMaxSpeed );
//
//	 mVelocity.Z = 0.f;
//
//	ActionConsumeMovementInputVector();
//}

void ARealTimeSrvPawn::ApplyControlInputToVelocity( float DeltaTime, FVector& refVelocity )
{
	const FVector ControlAcceleration = ActionGetPendingInputVector().GetClampedToMaxSize( 1.f );

	const float AnalogInputModifier = ( ControlAcceleration.SizeSquared() > 0.f ? ControlAcceleration.Size() : 0.f );
	const float MaxPawnSpeed = GetMaxSpeed() * AnalogInputModifier;
	const bool bExceedingMaxSpeed = IsExceedingMaxSpeed( MaxPawnSpeed );

	if ( AnalogInputModifier > 0.f && !bExceedingMaxSpeed )
	{
		// Apply change in velocity direction
		if ( refVelocity.SizeSquared() > 0.f )
		{
			// Change direction faster than only using acceleration, but never increase velocity magnitude.
			const float TimeScale = FMath::Clamp( DeltaTime * TurningBoost, 0.f, 1.f );
			refVelocity = refVelocity + ( ControlAcceleration * refVelocity.Size() - refVelocity ) * TimeScale;
		}
	}
	else
	{
		// Dampen velocity magnitude based on deceleration.
		if ( refVelocity.SizeSquared() > 0.f )
		{
			const FVector OldVelocity = refVelocity;
			const float VelSize = FMath::Max( refVelocity.Size() - FMath::Abs( Deceleration ) * DeltaTime, 0.f );
			refVelocity = refVelocity.GetSafeNormal() * VelSize;

			// Don't allow braking to lower us below max speed if we started above it.
			if ( bExceedingMaxSpeed && refVelocity.SizeSquared() < FMath::Square( MaxPawnSpeed ) )
			{
				refVelocity = OldVelocity.GetSafeNormal() * MaxPawnSpeed;
			}
		}
	}

	// Apply acceleration and clamp velocity magnitude.
	const float NewMaxSpeed = ( IsExceedingMaxSpeed( MaxPawnSpeed ) ) ? refVelocity.Size() : MaxPawnSpeed;
	refVelocity += ControlAcceleration * FMath::Abs( Acceleration ) * DeltaTime;
	refVelocity = refVelocity.GetClampedToMaxSize( NewMaxSpeed );

	 refVelocity.Z = 0.f;

	ActionConsumeMovementInputVector();
}

void ARealTimeSrvPawn::Read( InputBitStream& inInputStream )
{
	bool stateBit;

	uint32_t readState = 0;

	inInputStream.Read( stateBit );
	if ( stateBit )
	{
		uint32_t playerId;
		inInputStream.Read( playerId );
		if ( !IsAlreadySetPlayerID() )
		{
			SetPlayerId( playerId );
			mPlayerIDAlreadySetFlag = true;
			readState |= ECRS_PlayerId;

			R_LOG_M( "READ!!! GetPlayerId() = %d", GetPlayerId() );

			if ( GetPlayerId() == NetworkMgr::sInstance->GetPlayerId() )
			{
				APlayerController* const FirstPC = UGameplayStatics::GetPlayerController( GetWorld(), 0 );
				if ( FirstPC != nullptr )
				{
					FirstPC->Possess( this );

					GEngine->AddOnScreenDebugMessage( -1, 6.f, FColor::Red,
						FString::Printf( TEXT( "%s    %d" ),
							*FString( "Welcome on client as PlayerID     = " ), mPlayerId )
					);
				}
			}
			else
			{
				mIsTimeToStartSimulateMovementForRemotePawn =
					NetworkMgr::kTimeBufferStatePackets +
					RealTimeSrvTiming::sInstance->GetCurrentGameTime() -
					NetworkMgr::kTimeBetweenStatePackets;
			}
		}
	}

	//FRotator oldRotation = GetActorRotation();
	//FRotator oldActionPawnCameraRotation = RealTimeSrvPawnCamera->GetComponentRotation();
	FVector oldLocation = GetActorLocation();
	//FVector oldVelocity = GetLocalVelocity();


	R_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );


	R_LOG_M( "READ!!! GetActorRotation() = %f, %f, %f", GetActorRotation().Pitch, GetActorRotation().Yaw, GetActorRotation().Roll );
	R_LOG_M( "READ!!! mLocalRotation = %f, %f, %f", mLocalRotation.Pitch, mLocalRotation.Yaw, mLocalRotation.Roll );
	R_LOG_M( "READ!!! GetRotation() = %f, %f, %f", GetRotation().Pitch, GetRotation().Yaw, GetRotation().Roll );
	R_LOG_1( "------------------------" );

	R_LOG_M( "READ!!! mLocalActionPawnCameraRotation = %f, %f, %f", mLocalCameraRotation.Pitch, mLocalCameraRotation.Yaw, mLocalCameraRotation.Roll );
	R_LOG_M( "READ!!! ActionPawnCamera->GetComponentRotation() = %f, %f, %f", RealTimeSrvPawnCamera->GetComponentRotation().Pitch, RealTimeSrvPawnCamera->GetComponentRotation().Yaw, RealTimeSrvPawnCamera->GetComponentRotation().Roll );
	R_LOG_M( "READ!!! mActionPawnCameraRotation = %f, %f, %f", mCameraRotation.Pitch, mCameraRotation.Yaw, mCameraRotation.Roll );
	R_LOG_1( "------------------------" );

	R_LOG_M( "READ!!! GetActorLocation() = %f, %f, %f", GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z );
	R_LOG_M( "READ!!! mLocalLocation = %f, %f, %f", mLocalLocation.X, mLocalLocation.Y, mLocalLocation.Z );
	R_LOG_M( "READ!!! GetLocation() = %f, %f, %f", GetLocation().X, GetLocation().Y, GetLocation().Z );
	R_LOG_1( "------------------------" );

	R_LOG_M( "READ!!! mVelocity = %f, %f, %f", mVelocity.X, mVelocity.Y, mVelocity.Z );
	R_LOG_M( "READ!!! Velocity = %f, %f, %f", mLocalVelocity.X, mLocalVelocity.Y, mLocalVelocity.Z );

	R_LOG_1( "~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" );


	//FRotator replicatedRotation;
	//FVector replicatedLocation;
	//FVector replicatedVelocity;
	//FRotator replicatedActionPawnCameraRotation;

	inInputStream.Read( stateBit );
	if ( stateBit )
	{
		inInputStream.Read( mVelocity.X );
		inInputStream.Read( mVelocity.Y );
		inInputStream.Read( mVelocity.Z );

		//SetActionEntityVelocity( replicatedVelocity );

		inInputStream.Read( mLocation.X );
		inInputStream.Read( mLocation.Y );
		inInputStream.Read( mLocation.Z );

		//SetLocation( replicatedLocation );

		inInputStream.Read( mRotation.Pitch );
		inInputStream.Read( mRotation.Yaw );
		inInputStream.Read( mRotation.Roll );

		//SetRotation( replicatedRotation );

		inInputStream.Read( mCameraRotation.Pitch );
		inInputStream.Read( mCameraRotation.Yaw );
		inInputStream.Read( mCameraRotation.Roll );

		//SetActionPawnCameraRotation(mActionPawnCameraRotation);

		readState |= ECRS_Pose;
	}

	if ( ( readState & ECRS_PlayerId ) != 0 )
	{
		InitAfterCreate();
	}
	else
	{
		if ( GetPlayerId() == NetworkMgr::sInstance->GetPlayerId() )
		{
			ReplayForLocalPawn( readState );
			if ( oldLocation != GetLocation() )
			{
				bIsPlayerLocationOutOfSync = true;
			}
		}
		else
		{
			mStateBuffer.AddStateData( mRotation, mVelocity, mLocation, mCameraRotation );

			R_LOG_1_EXTRA( "===================mStateBuffer.AddStateData=======start===================" );
			R_LOG_M_EXTRA( "mStateBuffer.AddStateData.mLocation = %f, %f, %f", mLocation.X, mLocation.Y, mLocation.Z );
			R_LOG_M_EXTRA( "mStateBuffer.AddStateData.mVelocity = %f, %f, %f", mVelocity.X, mVelocity.Y, mVelocity.Z );
			R_LOG_M_EXTRA( "mRotation = %f, %f, %f", mRotation.Pitch, mRotation.Yaw, mRotation.Roll );
			R_LOG_M_EXTRA( "mCameraRotation = %f, %f, %f", mCameraRotation.Pitch, mCameraRotation.Yaw, mCameraRotation.Roll );
			R_LOG_1_EXTRA( "======end=============mStateBuffer.AddStateData==========================" );
		}
	}

}

void ARealTimeSrvPawn::InitAfterCreate()
{
	mLocalLocation = mLocation;
	mLocalVelocity = mVelocity;
	mLocalRotation = mRotation;
	mLocalCameraRotation = mCameraRotation;

	mRemotePawnTargetLocation = mLocation;
	mRemotePawnTargetVelocity = mVelocity;
	mRemotePawnTargetRotation = mRotation;
	mRemotePawnTargetCameraRotation = mCameraRotation;

	SetActorLocation( mLocalLocation );
	//SetLocalVelocity( mLocalVelocity );
	SetActorRotation( mLocalRotation );
	RealTimeSrvPawnCamera->SetWorldRotation( mLocalCameraRotation );
}

void ARealTimeSrvPawn::ReplayForLocalPawn( uint32_t inReadState )
{
	if ( ( inReadState & ECRS_Pose ) != 0 )
	{
		const ActionList& moveList = InputMgr::sInstance->GetActionList();

		for ( const Action& move : moveList )
		{
			float deltaTime = move.GetDeltaTime();
			ProcessInputBaseOnServerState( deltaTime, move.GetInputState() );
		}
	}
}

void ARealTimeSrvPawn::ReplayForRemotePawn( uint32_t inReadState )
{
	if ( ( inReadState & ECRS_Pose ) != 0 )
	{
		float rtt = NetworkMgr::sInstance->GetRoundTripTime();
		float deltaTime = 1.f / 30.f;
		while ( true )
		{
			if ( rtt < deltaTime )
			{
				DR( rtt );
				break;
			}
			else
			{
				DR( deltaTime );
				rtt -= deltaTime;
			}
		}
	}

}

