// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "BowArrow.generated.h"

/**
 * 
 */
UCLASS()
class TAXOFWAR_API ABowArrow : public AWeapon
{
	GENERATED_BODY()
	
public:
	ABowArrow();

	virtual void Equip(class AMainHero* MainHero) override;

	void ShootArrow();

private:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class USkeletalMeshComponent* SkeletalMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	USceneComponent* SpawnLocation;

	UPROPERTY(EditDefaultsOnly, category = "Combat")
	TSubclassOf<class AProjectile> ProjectileClass;
	
};
