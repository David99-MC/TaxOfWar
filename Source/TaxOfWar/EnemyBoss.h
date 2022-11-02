// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnemyBase.h"
#include "EnemyBoss.generated.h"

/**
 * 
 */
UCLASS()
class TAXOFWAR_API AEnemyBoss : public AEnemyBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;
	
public:

	AEnemyBoss();

	UPROPERTY(EditAnywhere, Category = "Animations")
	TArray<UAnimMontage*> LongAttackAnimations;

	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser);

protected:

	void StateChaseClose() override;
	void LongAttack(bool bShouldRotate = true);
	// void MoveForward();
	
private:
	// long range jump attack
	UPROPERTY(EditAnywhere, Category = "Combat")
	float LongAttack_Cooldown;
	float LongAttack_Timestamp;
	float LongAttack_ForwardSpeed;

	// after x consecutive hits, the enemy cannot be interrupted
	// int QuickHitsTaken;
	// float QuickHitsTimestamp;
	// UPROPERTY(EditAnywhere, Category = "Combat")
	// float HitThreshold;
};
