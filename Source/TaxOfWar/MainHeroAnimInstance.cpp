// Fill out your copyright notice in the Description page of Project Settings.


#include "MainHeroAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MainHero.h"

void UMainHeroAnimInstance::NativeInitializeAnimation()
{
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner();
        if (Pawn)
        {
            MainHero = Cast<AMainHero>(Pawn);
        }
    }    
}

void UMainHeroAnimInstance::UpdateAnimationProperties()
{
    if (Pawn == nullptr)
    {
        Pawn = TryGetPawnOwner();
    }
    else
    {
        if (MainHero == nullptr)
        {
            MainHero = Cast<AMainHero>(Pawn);
        }
    }
}