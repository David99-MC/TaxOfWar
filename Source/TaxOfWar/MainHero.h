// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combatant.h"
#include "MainHero.generated.h"

/**
 * 
 */
UENUM(BlueprintType)
enum class EWeaponType : uint8
{
	EWT_BareHand 		UMETA(DisplayName = "Bare hand"),
	EWT_TwoHandSword 	UMETA(DisplayName = "Two Hand Sword"),
	EWT_Spear			UMETA(DisplayName = "Spear"),
	EWT_BowArrow		UMETA(DisplayName = "Bow and Arrow"),

	EWT_MAX				UMETA(DisplayName = "DefaultMAX")

};

UENUM(BlueprintType)
enum class EMovementStatus : uint8
{
	EMS_Normal 		UMETA(DisplayName = "Normal"),
	EMS_Sprinting 	UMETA(DisplayName = "Sprinting"),
	EMS_Dead		UMETA(DisplayName = "Dead"),

	EMS_MAX 		UMETA(DisplayName = "DefaultMAX")
};

UENUM(BlueprintType)
enum class EStaminaStatus : uint8
{
	ESS_Normal   			UMETA(DisplayName = "Normal"),
	ESS_BelowMinimum 		UMETA(DisplayName = "BelowMinimum"),
	ESS_Exhausted 			UMETA(DisplayName = "Exhausted"),
	ESS_ExhaustedRecovery 	UMETA(DisplayName = "ExhaustedRecovery"),

	ESS_MAX 				UMETA(Displayname = "DefaultMAX") 
};

UCLASS()
class TAXOFWAR_API AMainHero : public ACombatant
{
	GENERATED_BODY()

public:
	AMainHero();

public: // Stamina involved section
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums");
	EMovementStatus MovementStatus;

	void SetMovementStatus(EMovementStatus Status);

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Enums");
	EStaminaStatus StaminaStatus;

	FORCEINLINE void SetStaminaStatus(EStaminaStatus Status) { StaminaStatus = Status; }

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Player Stats")
	float MaxStamina;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina;

	float SprintingDrainRate;
	float StaminaRegenRate;
	float MinSprintStamina;

	float SprintingSpeed;

	bool bShiftKeyDown;

	void ShiftKeyUp();
	void ShiftKeyDown();

	void Sprinting(float DeltaSecond);

	UFUNCTION(BlueprintCallable)
	void TakeStamina(float Amount);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float RollStamina;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float AttackStamina;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class USpringArmComponent* CameraBoom;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	class UCameraComponent* FollowCamera;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseTurnRate;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	float BaseLookUpRate;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float NormalSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float CombatMovementSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float RollSpeed;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Movement")
	float StumblingSpeed;

	bool bMoveForward;
	bool bMoveRight;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float CombatDistance;

	void CycleTarget(bool Clockwise = true);

	FRotator GetLookAtRotationYaw(FVector Target);

	float TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent,
	AController* EventInstigator, AActor* DamageCauser);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	class UAnimMontage* BareHandMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	UAnimMontage* TwoHandSwordMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	UAnimMontage* SpearMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Anims")
	UAnimMontage* BowArrowMontage;

	float AttackSpeed;
	FORCEINLINE void SetAttackSpeed(float Amount) { AttackSpeed = Amount; }

	bool Rolling;
	FRotator RollRotation;

	int AttackIndex;
	float TargetLockDistance;

	TArray<class AActor*> NearbyEnemies;
	int LastStumbleIndex;

	FVector InputDirection;

protected:

	void MoveForward(float Value);
	void MoveRight(float Value);

	void Attack();
	void EndAttack();

	void Roll();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void StartRoll();

	UFUNCTION(BlueprintCallable, Category = "Combat")
	void EndRoll();

	UFUNCTION(BlueprintCallable)
	bool GetRolling();

	void RollRotateSmooth();
	void FocusTarget();
	void ToggleCombatMode();
	void SetInCombat(bool InCombat);

	void TurnAtRate(float Rate);
	void LookUpAtRate(float Rate);

public:
	
	class USpringArmComponent* GetCameraBoom() const 
	{ 
		return CameraBoom; 
	}

	class UCameraComponent* GetFollowCamera() const
	{
		return FollowCamera;
	}

public:
	class UAnimInstance* AnimInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Enums")
	EWeaponType WeaponType;
	FORCEINLINE void SetWeaponType(EWeaponType NewWeaponType) { WeaponType = NewWeaponType; }

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	class AItem* ActiveOverlappingItem;
	FORCEINLINE void SetActiveOverlappingItem(AItem* Item) {ActiveOverlappingItem = Item;}

	void PickUpItem();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Items")
	class AWeapon* EquippedWeapon;
	void SetEquippedWeapon(AWeapon* WeaponToSet);

	virtual void SetAttackDamaging(bool Damaging) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float GetHitInterpSpeed;
	float InterpSpeed;
	bool bInterpToEnemy;
	void SetInterpToEnemy(bool Interp);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Combat")
	class AEnemyBase* CombatTarget;
	FORCEINLINE void SetCombatTarget(AEnemyBase* Enemy) { CombatTarget = Enemy; }
	
	UFUNCTION(BlueprintImplementableEvent)
	void ResetGame();

	UFUNCTION(BlueprintCallable)
	void Die();

	UFUNCTION(BlueprintCallable)
	void DeathEnd();

	void UpdateCombatTarget();
	UPROPERTY(EditAnywhere, Category = "Combat")
	TSubclassOf<AEnemyBase> EnemyFilter;

	UFUNCTION(BlueprintCallable)
	void Fire();

public: // Effects
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects | VFX")
	class UParticleSystem* GetHitParticles;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Effects | SFX")
	class USoundCue* GetHitSound;

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Player Stats", meta=(AllowPrivateAccess=true))
	class UAttributeComponent* Attributes;

};
