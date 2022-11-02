// Fill out your copyright notice in the Description page of Project Settings.


#include "Combatant.h"
#include "GameFramework/CharacterMovementComponent.h"

// Sets default values
ACombatant::ACombatant()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TargetLocked = false;
	NextAttackReady = false;
	Attacking = false;
	AttackDamaging = false;
	MovingForward = false;
	MovingBackwards = false;
	RotateTowardsTarget = false;
	Stumbling = false;
	RotationSmoothingRate = 10.f;
	LastRotationSpeed = 0.f;

	bIsAlive = true;

}

// Called when the game starts or when spawned
void ACombatant::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ACombatant::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (RotateTowardsTarget && bIsAlive)
		LookAtSmooth(DeltaTime);
	
}

// Called to bind functionality to input
void ACombatant::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ACombatant::Attack()
{
	Attacking = true;
	NextAttackReady = false;
	AttackDamaging = false;

	// Empty the array for the next attack
	AttackHitActors.Empty();
}

void ACombatant::AttackLunge()
{
	if (Target)
	{
		FVector Direction = Target->GetActorLocation() - GetActorLocation();
		Direction = FVector(Direction.X, Direction.Y, 0);
		
		// This gives the rotation towards the Target
		FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		SetActorRotation(Rotation);
	}

	// Propelling this Character forward, so we set bSweep = true
	FVector NewLocation = GetActorLocation() + (GetActorForwardVector() * JumpRange);
	SetActorLocation(NewLocation, true);
}

void ACombatant::EndAttack()
{
	Attacking = false;
	NextAttackReady = false;
}

void ACombatant::SetAttackDamaging(bool Damaging)
{
	AttackDamaging = Damaging; 
}

void ACombatant::SetMovingForward(bool IsMovingForward)
{
	MovingForward = IsMovingForward;
}

void ACombatant::SetMovingBackwards(bool IsMovingBackwards)
{
	MovingBackwards = IsMovingBackwards;
}

void ACombatant::EndStumble()
{
	Stumbling = false;
}

void ACombatant::AttackNextReady()
{
	NextAttackReady = true;
}

void ACombatant::LookAtSmooth(float Deltatime)
{
	if (Target && TargetLocked && !Attacking && !GetCharacterMovement()->IsFalling())
	{
		FVector Direction = Target->GetActorLocation() - GetActorLocation();
		Direction = FVector(Direction.X, Direction.Y, 0);
		
		// This gives the rotation towards the Target
		FRotator Rotation = FRotationMatrix::MakeFromX(Direction).Rotator();
		FRotator SmoothedRotation = FMath::RInterpTo(GetActorRotation(), Rotation, Deltatime, RotationSmoothingRate);

		SetActorRotation(SmoothedRotation);
		
		LastRotationSpeed = SmoothedRotation.Yaw - GetActorRotation().Yaw;
	}
}

float ACombatant::GetCurrentRotationSpeed()
{
	if (RotateTowardsTarget)
		return LastRotationSpeed;

	return 0.0f;
}

bool ACombatant::GetAttacking()
{
	return Attacking;
}

bool ACombatant::GetAttackDamaging()
{
	return AttackDamaging;
}

