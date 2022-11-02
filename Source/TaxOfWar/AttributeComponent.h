// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AttributeComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class TAXOFWAR_API UAttributeComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAttributeComponent();
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:	
	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float Health;

	UPROPERTY(EditAnywhere, Category = "Actor Attributes")
	float MaxHealth;

public:
	UFUNCTION(Blueprintcallable)
	void ReceiveDamage(float Damage);
	UFUNCTION(Blueprintcallable)
	void IncrementHealth(float Amount);
	
	UFUNCTION(BlueprintCallable)
	float GetHealthPercent();

	FORCEINLINE float GetHealth() {return Health;}
	FORCEINLINE float GetMaxHealth() {return MaxHealth;}
};
