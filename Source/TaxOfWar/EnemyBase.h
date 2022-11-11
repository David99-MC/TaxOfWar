// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combatant.h"
#include "EnemyBase.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class State : uint8
{
	IDLE,					// Outside of combat
	CHASE_CLOSE,			// Combat, staying close to target
	CHASE_FAR,				// Combat, doesn't care about range
	ATTACK,					// In the process of attacking
	STUMBLE,				// Stumbling from being damaged/interrupted
};

UCLASS()
class TAXOFWAR_API AEnemyBase : public ACombatant
{
	GENERATED_BODY()

public: // Combat
	AEnemyBase();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	UStaticMeshComponent* Weapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float JumpOffset;

	UPROPERTY(EditAnywhere, Category = "Combat")
	UAnimMontage* DeadAnimMontage;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class UBoxComponent* CombatCollision;
	UFUNCTION()
	void CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()	
	void CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	UBoxComponent* ShieldCollision;
	UFUNCTION()
	void ShieldCombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()	
	void ShieldCombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Combat")
	class USphereComponent* AggroSphere;
	UFUNCTION()
	void AggroOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	UFUNCTION()	
	void AggroOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	UFUNCTION(BlueprintCallable)
	void ActivateCollision();

	UFUNCTION(BlueprintCallable)
	void DeactivateCollision();

	UFUNCTION(BlueprintCallable)
	void ActivateShieldCollision();

	UFUNCTION(BlueprintCallable)
	void DeactivateShieldCollision();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	FTimerHandle DeathTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float DeathDelay;

	void Disappear();

	bool bShouldRotateTowardsTarget;
	
	UPROPERTY(VisibleAnywhere, Category = "Combat")
	bool bHasValidTarget;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Finite State Machine")
	State ActiveState;

	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser) override;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float CloseAttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Attack_Cooldown;
	float Attack_Timestamp;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float FarAttackRange;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Long_Attack_Cooldown;
	float Long_Attack_Timestamp;

protected:
	virtual void BeginPlay() override;

	virtual void TickStateMachine();

	void SetState(State NewState);

	virtual void StateIdle();

	// state: actively trying to keep close and attack the target
	virtual void StateChaseClose();

	virtual void StateAttack();

	virtual void StateStumble();

	void Die(AActor* Causer);

	virtual void MoveForward(float MoveDistance);

	virtual void Attack() override;

	void AttackNextReady();

	void EndAttack();

	virtual void AttackLunge();

	bool Interruptable;

public:
	virtual void Tick(float DeltaTime) override;

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void FocusTarget();

private:
	/*
	* Navigation
	*/
	class AAIController* AIController;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	AActor* PatrolTarget;

	UPROPERTY(EditAnywhere, Category = "AI Navigation")
	TArray<AActor*> PatrolTargets;

public: // Visual and Sound Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | VFX")
	class UParticleSystem* GetHitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | SFX")
	class USoundCue* GetHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | SFX")
	USoundCue* SwingSound;

protected: // for mobs, miniboss, witch
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enemy Stats")
	class UAttributeComponent* Attributes;

	UPROPERTY(VisibleAnywhere)
	class UHealthBarComponent* HealthBarWidget;

private: // Ranged Combat
	UPROPERTY(EditAnywhere, Category = "Animations")
	TArray<UAnimMontage*> RangedAttackAnimations;

	void RangedAttack(bool bShouldRotate);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* ProjectileSpawnLocation;

	UPROPERTY(EditDefaultsOnly, category = "Combat | Ranged")
	TSubclassOf<class AProjectile> ProjectileClass; // this stores a UClass type object

	void Teleport();
	FTimerHandle TeleportTimer;
	UPROPERTY(EditAnywhere, Category = "Combat | Ranged")
	float TeleportDelay;

	UPROPERTY(EditAnywhere, Category = "Combat | Ranged")
	float TeleportRange;

	UFUNCTION(BlueprintCallable)
	void Fire();

private:

	UPROPERTY(EditAnywhere, Category = "Combat | Boss")
	bool bIsBoss;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float CloseCombatDamage;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat", meta = (AllowPrivateAccess = "true"))
	float RangedCombatDamage;

	float CombatDamage;
	FORCEINLINE void SetCombatDamage(float Amount) {CombatDamage = Amount;}

	int QuickHitsTaken;
	float QuickHitsTimestamp;

	UPROPERTY(EditAnywhere, Category = "Combat")
	float HitThreshold;

	UPROPERTY(EditAnywhere, Category = "Combat | Boss")
	float CloseAttackSpeedPercent;

	UPROPERTY(EditAnywhere, Category = "Combat | Boss")
	float RangedAttackSpeedPercent;

	UPROPERTY(EditAnywhere, Category = "Combat | Boss")
	float HealthPercentThreshold;

	UPROPERTY(EditAnywhere, Category = "Combat | Boss")
	float DamageMultiplier;
	FORCEINLINE void SetDamageMultiplier(float Amount) { DamageMultiplier = Amount; }

	UPROPERTY(EditAnywhere, Category = "Combat | Boss")
	UAnimMontage* TauntingAnimMontage;
	
	bool bHasEvolved = false;
	bool bEvolving = false;

	UPROPERTY(EditAnywhere, Category = "Combat | Boss")
	float ReducedDamagePercent;

	UFUNCTION(BlueprintCallable)
	void StartTaunt();

	UFUNCTION(BlueprintCallable)
	void EndTaunt();
};
