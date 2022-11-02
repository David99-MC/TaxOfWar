// Fill out your copyright notice in the Description page of Project Settings.


#include "Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Components/SphereComponent.h"
#include "MainHero.h"
#include "Sound/SoundCue.h"

// Sets default values
AProjectile::AProjectile()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	ProjectileMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Projectile Mesh"));
	RootComponent = ProjectileMesh;

	HitBox = CreateDefaultSubobject<USphereComponent>(TEXT("Hit Box"));
	HitBox->SetupAttachment(RootComponent);

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
	HitBox->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::HitBoxOnOverlapBegin);
    HitBox->OnComponentEndOverlap.AddDynamic(this, &AProjectile::HitBoxOnOverlapEnd);
	SetLifeSpan(5.f);
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

	if (HitParticle && HitMuzzle)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, HitParticle, GetActorLocation(), GetActorRotation());
		UGameplayStatics::SpawnEmitterAtLocation(this, HitMuzzle, GetActorLocation(), GetActorRotation());
		if (HitSound)
			UGameplayStatics::PlaySoundAtLocation(this, HitSound, GetActorLocation());
	}
	// if OtherActor is not this particular projectile and its owner
	AMainHero* MainHero = Cast<AMainHero>(OtherActor);
	if (MainHero && OtherActor != this && OtherActor != MyOwner)  
	{
		UGameplayStatics::ApplyDamage(OtherActor, ProjectileDamage, MyOwnerInstigator, this, UDamageType::StaticClass());
		if (MainHero->GetHitSound)
			UGameplayStatics::PlaySound2D(this, MainHero->GetHitSound, 0.5f);
	}	
	Destroy();
}

void AProjectile::HitBoxOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{

}


