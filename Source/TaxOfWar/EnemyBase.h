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
	float CloseAttackRange;

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

	class AAIController* AIController;

	FTimerHandle AttackTimer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float Attack_Cooldown;
	float Attack_Timestamp;

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

public: // Visual and Sound Effects
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | VFX")
	class UParticleSystem* GetHitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | SFX")
	class USoundCue* GetHitSound;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Effects | SFX")
	USoundCue* SwingSound;

protected: // for mobs, miniboss, witch
	UPROPERTY(VisibleAnywhere)
	class UAttributeComponent* Attributes;

	UPROPERTY(VisibleAnywhere)
	class UHealthBarComponent* HealthBarWidget;

private: // Ranged Combat
	UPROPERTY(EditAnywhere, Category = "Animations")
	TArray<UAnimMontage*> RangedAttackAnimations;

	void RangedAttack(bool bShouldRotate);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* ProjectileLocation;

	UPROPERTY(EditDefaultsOnly, category = "Combat")
	TSubclassOf<class AProjectile> ProjectileClass; // this stores a UClass type object

	void Teleport();
	FTimerHandle TeleportTimer;
	UPROPERTY(EditAnywhere, Category = "Combat | Ranged")
	float TeleportDelay;

	UPROPERTY(EditAnywhere, Category = "Combat | Ranged")
	float TeleportRange;

	UFUNCTION(BlueprintCallable)
	void Fire();

	UFUNCTION(BlueprintCallable)
	void FireSpecial();

	// Spawning enemies
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Ranged", meta = (AllowPrivateAccess = "true"))
	TSet<AEnemyBase*> Minions;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat | Ranged", meta = (AllowPrivateAccess = "true"))
	TSet<FVector> EnemySpawningLocations;

private:
	int QuickHitsTaken;
	float QuickHitsTimestamp;
	UPROPERTY(EditAnywhere, Category = "Combat")
	float HitThreshold;
	UPROPERTY(EditAnywhere, Category = "Combat")
	bool bIsBoss;

};
