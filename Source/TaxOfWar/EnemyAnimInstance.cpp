// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyAnimInstance.h"
#include "EnemyBase.h"

void UEnemyAnimInstance::NativeInitializeAnimation()
{
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            EnemyBase = Cast<AEnemyBase>(Pawn);
        }
    }    
}

void UEnemyAnimInstance::UpdateAnimationProperties()
{
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner();
    }
    else
    {
        FVector Speed = Pawn->GetVelocity();
        FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0);
        MovementSpeed = LateralSpeed.Size();

        if (EnemyBase == nullptr)
        {
            EnemyBase = Cast<AEnemyBase>(Pawn);
        }
    }
}