// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBoss.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "AttributeComponent.h"
#include "HealthBarComponent.h"


AEnemyBoss::AEnemyBoss()
{
    // HitThreshold = 3;
}

void AEnemyBoss::BeginPlay()
{
    LongAttack_Timestamp = -LongAttack_Cooldown;
    HealthBarWidget->SetHealthPercent(Attributes->GetHealthPercent());
}

void AEnemyBoss::StateChaseClose()
{
    // float Distance = FVector::Distance(Target->GetActorLocation(),GetActorLocation());

    // FVector TargetDirection = Target->GetActorLocation() - GetActorLocation();
    // float DotProduct = FVector::DotProduct(GetActorForwardVector(), TargetDirection.GetSafeNormal());

    // if (Distance <= AttackRange)
    // {
    //     bHasValidTarget = true;
    //     if (DotProduct > 0.5f)
    //         bShouldRotateTowardsTarget = false;
    //     else
    //         bShouldRotateTowardsTarget = true;
    //     // delay for normal attacks
    //     if (UGameplayStatics::GetTimeSeconds(GetWorld()) >= (Attack_Timestamp + Attack_Cooldown))
    //     {
    //         Attack_Timestamp = UGameplayStatics::GetTimeSeconds(GetWorld());
    //         Attack();
    //     }
    //     return;
    // }
    // else // out of close range attack, starting long range attack
    // {
    //     if (UGameplayStatics::GetTimeSeconds(GetWorld()) >= (LongAttack_Timestamp + LongAttack_Cooldown) 
    //         && AIController->LineOfSightTo(Target))
    //         {
    //             // Set the time stamp for the next long range attack
    //             LongAttack_Timestamp = UGameplayStatics::GetTimeSeconds(GetWorld());
    //             LongAttack(true);
    //             return;
    //         }
    // }
    // if (!AIController->IsFollowingAPath())
	// {
	// 	AIController->MoveToActor(Target);
	// }
}   

void AEnemyBoss::LongAttack(bool bShouldRotate)
{
    Super::Attack();

    SetMovingBackwards(false);
    SetMovingForward(false);

    if (AIController)
        AIController->StopMovement();
    
    if (bShouldRotate)
    {
        FVector TargetDirection = Target->GetActorLocation() - GetActorLocation();
        FVector TargetDirection2D = FVector(TargetDirection.X, TargetDirection.Y, 0);

        FRotator Rotation = FRotationMatrix::MakeFromX(TargetDirection2D).Rotator();
        SetActorRotation(Rotation);
    }

    SetState(State::ATTACK);

    int RandomIndex = FMath::RandRange(0, LongAttackAnimations.Num() - 1);
    PlayAnimMontage(LongAttackAnimations[RandomIndex], 0.65f);
}

float AEnemyBoss::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    // if (DamageCauser == this)
    //     return 0.0f;
    
    // if (QuickHitsTaken == 0 || GetWorld()->GetTimeSeconds() - QuickHitsTimestamp <= 1.f)
    // {
    //     QuickHitsTaken++;
    //     QuickHitsTimestamp = GetWorld()->GetTimeSeconds();

    //     if (QuickHitsTaken >= HitThreshold)
    //         Interruptable = false;
    // }
    // else
    // {
    //     QuickHitsTaken = 0;
    //     Interruptable = false;
    // }

	return Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
}

