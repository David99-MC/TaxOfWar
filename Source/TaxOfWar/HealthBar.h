// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"

#include "HealthBar.generated.h"

/**
 * 
 */
UCLASS()
class TAXOFWAR_API UHealthBar : public UUserWidget
{
	GENERATED_BODY()

public:
	// setting the meta will link the c++ variable to the blueprint variable
	UPROPERTY(meta = (BindWidget)) 
	class UProgressBar* HealthBar; // the names in c++ and blueprint MUST be exactly the same
};
