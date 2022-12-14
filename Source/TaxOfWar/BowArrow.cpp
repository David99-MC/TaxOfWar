// Fill out your copyright notice in the Description page of Project Settings.


#include "BowArrow.h"
#include "MainHero.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"
#include "Projectile.h"

ABowArrow::ABowArrow()
{
    SkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
    SkeletalMesh->SetupAttachment(RootComponent);

    SpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Spawn point"));
    SpawnLocation->SetupAttachment(SkeletalMesh);
}

void ABowArrow::Equip(AMainHero* MainHero)
{
    if (MainHero)
    {
        Super::Equip(MainHero);
        const USkeletalMeshSocket* RighHandSocket = MainHero->GetMesh()->GetSocketByName("Weapon_L");
        if (RighHandSocket)
        {
            // Attach ME to the character (MainHero)
            RighHandSocket->AttachActor(this, MainHero->GetMesh());
            MainHero->SetWeaponType(EWeaponType::EWT_BowArrow);
            MainHero->SetAttackSpeed(BowArrow_AttackSpeed);
            if (EquipSound)
            {
                UGameplayStatics::PlaySound2D(this, EquipSound);
            }
        }

    }
}

void ABowArrow::ShootArrow()
{
    AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
		ProjectileClass,
		SpawnLocation->GetComponentLocation(),
		SpawnLocation->GetComponentRotation() );
	    // projectileClass is a UClass object which is a blueprint based on C++ Projectile class
    if (Projectile == nullptr) return;
	Projectile->SetOwner(GetOwner());
}
