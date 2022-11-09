// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "TwoHandSword.generated.h"

/**
 * 
 */
UCLASS()
class TAXOFWAR_API ATwoHandSword : public AWeapon
{
	GENERATED_BODY()
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay();
public:
	virtual void Equip(class AMainHero* MainHero) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float THSDamge;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float THS_AttackSpeed;

};
