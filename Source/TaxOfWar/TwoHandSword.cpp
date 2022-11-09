// Fill out your copyright notice in the Description page of Project Settings.


#include "TwoHandSword.h"
#include "MainHero.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

void ATwoHandSword::BeginPlay()
{
    Super::BeginPlay();
}

void ATwoHandSword::Equip(AMainHero* MainHero)
{
    if (MainHero)
    {
        Super::Equip(MainHero);
        const USkeletalMeshSocket* RighHandSocket = MainHero->GetMesh()->GetSocketByName("Weapon_R");
        if (RighHandSocket)
        {
            // Attach ME to the character (MainHero)
            RighHandSocket->AttachActor(this, MainHero->GetMesh());
            MainHero->SetWeaponType(EWeaponType::EWT_TwoHandSword);
            SetWeaponDamage(THSDamge);
            MainHero->SetAttackSpeed(THS_AttackSpeed);
            if (EquipSound)
            {
                UGameplayStatics::PlaySound2D(this, EquipSound);
            }
        }

    }
}

