// Fill out your copyright notice in the Description page of Project Settings.


#include "Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/BoxComponent.h"
#include "MainHero.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "EnemyBase.h"
#include "NiagaraComponent.h"

AWeapon::AWeapon()
{
    HitBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Hit Box"));
    HitBox->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
    Super::BeginPlay();
    HitBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::HitBoxOnOverlapBegin);
    HitBox->OnComponentEndOverlap.AddDynamic(this, &AWeapon::HitBoxOnOverlapEnd);

    // Setting up Collision info
    HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    HitBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    HitBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    HitBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    Player = Cast<AMainHero>(UGameplayStatics::GetPlayerPawn(GetWorld(), 0));
    // Mesh->SetSimulatePhysics(true);
}

void AWeapon::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) 
{
    Super::OnOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
    if (OtherActor)
    {
        AMainHero* Hero = Cast<AMainHero>(OtherActor);
        if (Hero)
        {
            Hero->SetActiveOverlappingItem(this);
        }
    }
}

void AWeapon::OnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex) 
{
    Super::OnOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
    if (OtherActor)
    {
        AMainHero* MainHero = Cast<AMainHero>(OtherActor);
        if (MainHero)
        {
            MainHero->SetActiveOverlappingItem(nullptr);
        }
    }
}

void AWeapon::Equip(AMainHero* MainHero)
{
    if (MainHero)
    {
        SetInstigator(MainHero->GetController());
        SetOwner(MainHero);
        FinishFindMe(MainHero);

        SetActorTickEnabled(false);

        Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
        Mesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
        Mesh->SetSimulatePhysics(false);

        MainHero->SetEquippedWeapon(this);
        MainHero->SetActiveOverlappingItem(nullptr);
        
        if (IdleParticlesComponent)
            IdleParticlesComponent->Deactivate();
        
        if (IdleNiagaraComponent)
            IdleNiagaraComponent->Deactivate();
        
    }
}

void AWeapon::HitBoxOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor)
    {
        AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
        if (Enemy)
        {
            if (Enemy->GetHitParticles)
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Enemy->GetHitParticles, Enemy->GetActorLocation(), FRotator(0.f), false);
            if (Enemy->GetHitSound)
                UGameplayStatics::PlaySound2D(this, Enemy->GetHitSound);
            
            // Dealing damage to the enemy
            UGameplayStatics::ApplyDamage(Enemy, WeaponDamage, WeaponInstigator, this, UDamageType::StaticClass());
        }
    }
}	

void AWeapon::HitBoxOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    
}

void AWeapon::ActivateCollision()
{
    HitBox->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}
	
void AWeapon::DeactivateCollision()
{
    HitBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}
