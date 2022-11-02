// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "MainHeroPlayerController.generated.h"

/**
 * 
 */
UCLASS()
class TAXOFWAR_API AMainHeroPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	// Reference to the UMG asset in the editor
	// we need to to satisfy the parameter in CreateUserWidget
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<class UUserWidget> HUDOverlayAsset;

	// Variable to hold the widget after creating it
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	UUserWidget* HUDOverlay;

protected:
	virtual void BeginPlay() override;
	
};
