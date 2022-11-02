// Fill out your copyright notice in the Description page of Project Settings.


#include "MainHeroPlayerController.h"
#include "Blueprint/UserWidget.h"

void AMainHeroPlayerController::BeginPlay()
{
    Super::BeginPlay();
    if (HUDOverlayAsset)
        HUDOverlay = CreateWidget<UUserWidget>(this, HUDOverlayAsset);
    
    HUDOverlay->AddToViewport();
    HUDOverlay->SetVisibility(ESlateVisibility::Visible);

}