// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Weapon.h"
#include "Spear.generated.h"

/**
 * 
 */
UCLASS()
class TAXOFWAR_API ASpear : public AWeapon
{
	GENERATED_BODY()

public:
	virtual void Equip(class AMainHero* MainHero) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat")
	float SpearDamge;
	
};
