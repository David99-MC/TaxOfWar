// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/BoxComponent.h"
#include "MainHero.h"
#include "Sound/SoundCue.h"
#include "EnemyBase.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	// ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	// ProjectileMesh->SetupAttachment(RootComponent);

	BowSkeletalMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Skeletal Mesh"));
    BowSkeletalMesh->SetupAttachment(RootComponent);
	RootComponent = BowSkeletalMesh;

	Hit_Box = CreateDefaultSubobject<UBoxComponent>(TEXT("Hit Box"));
	Hit_Box->SetupAttachment(RootComponent);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement Component"));

	SmokeTrailParticle = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Smoke Trail Particle"));
	SmokeTrailParticle->SetupAttachment(RootComponent);
}

// Called when the game starts or when spawned
void AProjectile::BeginPlay()
{
	Super::BeginPlay();

	Target = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);
	
	UGameplayStatics::PlaySoundAtLocation(this, LaunchSound, GetActorLocation());
	Hit_Box->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::HitBoxOnOverlapBegin);
    Hit_Box->OnComponentEndOverlap.AddDynamic(this, &AProjectile::HitBoxOnOverlapEnd);
	SetLifeSpan(3.f);
}

// Called every frame
void AProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AProjectile::HitBoxOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	AActor* MyOwner = GetOwner();
	if (MyOwner == NULL) 
	{
		Destroy();
		return;
	}
	AController* MyOwnerInstigator = MyOwner->GetInstigatorController();

	// if OtherActor is not this particular projectile and its owner
	if (Cast<AEnemyBase>(GetOwner())) // Spawned by Enemies
	{
		AMainHero* MainHero = Cast<AMainHero>(OtherActor);
		if (MainHero && OtherActor != this && OtherActor != MyOwner)  
		{
			UGameplayStatics::ApplyDamage(MainHero, ProjectileDamage, MyOwnerInstigator, this, UDamageType::StaticClass());
			if (MainHero->GetHitSound)
				UGameplayStatics::PlaySound2D(this, MainHero->GetHitSound, 0.5f);
		}
	}	
	else // Spawned by Player
	{
		AEnemyBase* Enemy = Cast<AEnemyBase>(OtherActor);
		if (Enemy && OtherActor != this && OtherActor != MyOwner)  
		{
			UGameplayStatics::ApplyDamage(Enemy, ProjectileDamage, MyOwnerInstigator, this, UDamageType::StaticClass());
			if (Enemy->GetHitSound)
				UGameplayStatics::PlaySound2D(this, Enemy->GetHitSound, 0.5f);
		}
	}
	if (HitParticle && HitMuzzle) // Play effects
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, HitParticle, GetActorLocation(), GetActorRotation());
		UGameplayStatics::SpawnEmitterAtLocation(this, HitMuzzle, GetActorLocation(), GetActorRotation());
		if (HitSound)
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}

	Destroy();
}

void AProjectile::HitBoxOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}


