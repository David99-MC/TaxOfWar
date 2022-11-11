// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyBase.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "MainHero.h"
#include "Sound/SoundCue.h"
#include "Components/CapsuleComponent.h"
#include "TimerManager.h"
#include "AttributeComponent.h"
#include "HealthBarComponent.h"
#include "Projectile.h"

AEnemyBase::AEnemyBase()
{
    PrimaryActorTick.bCanEverTick = true;

    Weapon = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Weapon"));
    Weapon->SetupAttachment(GetMesh(), "Weapon_R");
    Weapon->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);

    ProjectileSpawnLocation = CreateDefaultSubobject<USceneComponent>(TEXT("Spawn point"));
    ProjectileSpawnLocation->SetupAttachment(Weapon);

    CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Combat Collision"));
    CombatCollision->SetupAttachment(GetMesh(), FName("EnemySocket"));

    ShieldCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("Shield Collision"));
    ShieldCollision->SetupAttachment(GetMesh(), FName("ShieldSocket"));

    AggroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Aggro Trigger"));
    AggroSphere->SetupAttachment(RootComponent);
    AggroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);

    // doesn't have to setup attachment since it doesn't have a mesh or a transform
    Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attribute Component"));

    HealthBarWidget = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
    HealthBarWidget->SetupAttachment(RootComponent);
    
    MovingForward = false;
    MovingBackwards = false;
    Interruptable = true;

    ActiveState = State::IDLE;

    DeathDelay = 3.f;
    HitThreshold = 3;
    QuickHitsTaken = 0;

    bShouldRotateTowardsTarget = false;
    bHasValidTarget = false;
}

void AEnemyBase::BeginPlay()
{
    Super::BeginPlay();

    Target = UGameplayStatics::GetPlayerPawn(GetWorld(), 0);

    CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::CombatOnOverlapBegin);
    CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemyBase::CombatOnOverlapEnd);
    // Setting up CombatCollision info
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    ShieldCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::ShieldCombatOnOverlapBegin);
    ShieldCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemyBase::ShieldCombatOnOverlapEnd);
    // Setting up ShieldCollision info
    ShieldCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    ShieldCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    ShieldCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    ShieldCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

    AggroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyBase::AggroOnOverlapBegin);
    AggroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemyBase::AggroOnOverlapEnd);

    AIController = Cast<AAIController>(Controller);

    GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
    GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

    ActiveState = State::IDLE;

    HealthBarWidget->SetHealthPercent(Attributes->GetHealth() / Attributes->GetMaxHealth());

    Attack_Timestamp = -Attack_Cooldown;
    Long_Attack_Timestamp = -Long_Attack_Cooldown;

    HealthBarWidget->SetVisibility(false);
}

void AEnemyBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    TickStateMachine();
}

void AEnemyBase::TickStateMachine()
{
    if (!bIsAlive) return;
    DrawDebugSphere(GetWorld(), GetActorLocation(), CloseAttackRange, 20, FColor::Red);
    DrawDebugSphere(GetWorld(), GetActorLocation(), FarAttackRange, 20, FColor::Blue);
    switch (ActiveState)
    {
        case State::IDLE:
            StateIdle();
            break;
        case State::CHASE_CLOSE:
            StateChaseClose();
            break;
        case State::ATTACK:
            StateAttack();
            break;
        case State::STUMBLE:
            StateStumble();
            break;
        default:
            break;
    }
}

void AEnemyBase::SetState(State NewState)
{
    ActiveState = NewState;
}

void AEnemyBase::StateIdle()
{
    if (AIController)
        AIController->StopMovement();
}

void AEnemyBase::StateChaseClose()
{
    AMainHero* Player = Cast<AMainHero>(Target);
    if (Player->MovementStatus == EMovementStatus::EMS_Dead)
    {
        bHasValidTarget = false;
        SetState(State::IDLE);
        return;
    }

    float Distance = FVector::Distance(Target->GetActorLocation(), GetActorLocation());
    if (Distance <= CloseAttackRange) // Get in range
    {
        if (bIsBoss)
        {
            GetWorldTimerManager().SetTimer(TeleportTimer, this, &AEnemyBase::Teleport, TeleportDelay);
        }

        bHasValidTarget = true;
        // Use DotProduct to hit the player in the correct direction
        FVector TargetDirection = Target->GetActorLocation() - GetActorLocation();
        float DotProduct = FVector::DotProduct(GetActorForwardVector(), TargetDirection.GetSafeNormal());

        if (DotProduct > 1.f && !Attacking && !Stumbling)
            bShouldRotateTowardsTarget = false;
        else
            bShouldRotateTowardsTarget = true;

        if (AIController)
            AIController->StopMovement();

        // Delay the Enemy attack
        if (UGameplayStatics::GetTimeSeconds(GetWorld()) >= (Attack_Timestamp + Attack_Cooldown))
        {
            Attack_Timestamp = UGameplayStatics::GetTimeSeconds(GetWorld());
            // Long_Attack_Timestamp = UGameplayStatics::GetTimeSeconds(GetWorld());
            
            Attack();
            return;
        }
    }
    else if (Distance > CloseAttackRange && Distance <= FarAttackRange) // Out of CloseAttackRange
    {
        bHasValidTarget = true;
        // Making a close or range attack will reset the the attack timer for both to prevent it from attacking twice
        if (UGameplayStatics::GetTimeSeconds(GetWorld()) >= (Long_Attack_Timestamp + Long_Attack_Cooldown) && AIController->LineOfSightTo(Target))
        {
            Long_Attack_Timestamp = UGameplayStatics::GetTimeSeconds(GetWorld());
            // Attack_Timestamp = UGameplayStatics::GetTimeSeconds(GetWorld());
            
            RangedAttack(true);
            return;
        }
    }
    else
    {
        if (bIsBoss)
            GetWorldTimerManager().ClearTimer(TeleportTimer);
    }

    if (AIController && !AIController->IsFollowingAPath())
        AIController->MoveToActor(Target);       
}

void AEnemyBase::Attack()
{
    if (!Attacking && bIsAlive && bHasValidTarget)
    {
        Super::Attack();
        
        SetMovingBackwards(false);
        SetMovingForward(false);
        SetCombatDamage(CloseCombatDamage);
        SetState(State::ATTACK);
        
        if (bShouldRotateTowardsTarget)
        {
            FVector TargetDirection = Target->GetActorLocation() - GetActorLocation();
            FVector TargetDirection2D = FVector(TargetDirection.X, TargetDirection.Y, 0);
            FRotator Rotation = FRotationMatrix::MakeFromX(TargetDirection2D).Rotator();
            SetActorRotation(Rotation); 
        }
        
        if (SwingSound)
            UGameplayStatics::PlaySound2D(this, SwingSound);

        int RandomIndex = FMath::RandRange(0, AttackAnimations.Num() - 1);
        PlayAnimMontage(AttackAnimations[RandomIndex]);
    }
}

void AEnemyBase::RangedAttack(bool bShouldRotate)
{
    if (!Attacking && bIsAlive && bHasValidTarget)
    {
        Super::Attack();
        SetMovingBackwards(false);
        SetMovingForward(false);
        SetCombatDamage(RangedCombatDamage);
        SetState(State::ATTACK);

        if (AIController)
            AIController->StopMovement();
        
        if (bShouldRotate)
        {
            FVector TargetDirection = Target->GetActorLocation() - GetActorLocation();
            FVector TargetDirection2D = FVector(TargetDirection.X, TargetDirection.Y, 0);
            FRotator Rotation = FRotationMatrix::MakeFromX(TargetDirection2D).Rotator();
            SetActorRotation(Rotation);
        }
        
        int RandomIndex = FMath::RandRange(0, RangedAttackAnimations.Num() - 1);
        PlayAnimMontage(RangedAttackAnimations[RandomIndex], .8f);
    }
}

void AEnemyBase::Teleport()
{
    SetActorLocation( GetActorLocation() + (FVector(0, GetActorLocation().Y + TeleportRange, 0)));
}

void AEnemyBase::AttackNextReady()
{
    Super::AttackNextReady();
}

void AEnemyBase::EndAttack()
{
    Super::EndAttack();
    SetState(State::CHASE_CLOSE);
}

void AEnemyBase::AttackLunge()
{
    Super::AttackLunge();
}

void AEnemyBase::StateAttack() // Enemy will use JumpOffset
{
    if (MovingForward) // AnimNotify will trigger this
        MoveForward(JumpOffset);
}

void AEnemyBase::MoveForward(float MoveDistance)
{
    float Distance = FVector::Distance(Target->GetActorLocation(),GetActorLocation());
    float LongAttack_ForwardSpeed = Distance + MoveDistance;
    FVector NewLocation = GetActorLocation() + 
                         (GetActorForwardVector() * LongAttack_ForwardSpeed * GetWorld()->GetDeltaSeconds());
    SetActorLocation(NewLocation, true);
}

void AEnemyBase::StateStumble()
{
    if (Stumbling)
        AddMovementInput(-GetActorForwardVector(), 20.f * GetWorld()->GetDeltaSeconds());
    else
        SetState(State::CHASE_CLOSE);
}

void AEnemyBase::FocusTarget()
{
    if (AIController)
        AIController->SetFocus(Target);
}

float AEnemyBase::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (DamageCauser == this)
        return 0.0f;
    if (bEvolving)
        return 0.0f;

    if (QuickHitsTaken == 0 || GetWorld()->GetTimeSeconds() - QuickHitsTimestamp <= 2.f)
    {
        QuickHitsTaken++;
        QuickHitsTimestamp = GetWorld()->GetTimeSeconds();

        if (QuickHitsTaken >= HitThreshold)
            Interruptable = false;
    }
    else
    {
        QuickHitsTaken = 0;
        Interruptable = true;
    }

    if (Interruptable)
    {
        EndAttack();
        SetMovingForward(false);
        SetMovingBackwards(false);
        Stumbling = true;

        SetState(State::STUMBLE);
        if (AIController)
            AIController->StopMovement();    

        PlayAnimMontage(TakeHit_StumbleBackwards[0]);
    }
    
    if (Attributes)
    {
        Attributes->ReceiveDamage(DamageAmount);
        if (HealthBarWidget)
        {
            float RemainingHealth = Attributes->GetHealthPercent(); 
            HealthBarWidget->SetHealthPercent(RemainingHealth);
            if (RemainingHealth <= 0)
            {
                SetActorEnableCollision(false);
                Die(DamageCauser);
            }
            else if (!bHasEvolved && RemainingHealth <= HealthPercentThreshold && TauntingAnimMontage)
            {
                bHasEvolved = true;
                PlayAnimMontage(TauntingAnimMontage);
            }
        }
    }
	return DamageAmount;
}

void AEnemyBase::Die(AActor* Causer)
{
    bIsAlive = false;
    Target = nullptr;
    PlayAnimMontage(DeadAnimMontage);

    CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    AggroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    AMainHero* MainHero = Cast<AMainHero>(Causer);
    if (MainHero)
        MainHero->UpdateCombatTarget();
}

void AEnemyBase::CombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // This is the CombatCollision for the Weapon
    if (OtherActor)
    {
        AMainHero* MainHero = Cast<AMainHero>(OtherActor);
        if (MainHero && MainHero->MovementStatus != EMovementStatus::EMS_Dead && bHasValidTarget)
        {
            if (MainHero->Rolling) // can't deal damage when the player is rolling
                return;      
            if (MainHero->GetHitParticles)
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MainHero->GetHitParticles, MainHero->GetActorLocation(), FRotator(0.f), false);
            if (MainHero->GetHitSound)
                UGameplayStatics::PlaySound2D(this, MainHero->GetHitSound, 0.6f);

            float NewDamage = CombatDamage;
            if (bHasEvolved)
                NewDamage = CombatDamage * DamageMultiplier;
            UGameplayStatics::ApplyDamage(MainHero, NewDamage, AIController, this, UDamageType::StaticClass());
        }
    }
}	

void AEnemyBase::CombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    
}

void AEnemyBase::ShieldCombatOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // This is the CombatCollision for the Weapon
    if (OtherActor)
    {
        AMainHero* MainHero = Cast<AMainHero>(OtherActor);
        if (MainHero && MainHero->MovementStatus != EMovementStatus::EMS_Dead && bHasValidTarget)
        {
            if (MainHero->Rolling) // can't deal damage when the player is rolling
                return;      
            if (MainHero->GetHitParticles)
                UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), MainHero->GetHitParticles, MainHero->GetActorLocation(), FRotator(0.f), false);
            if (MainHero->GetHitSound)
                UGameplayStatics::PlaySound2D(this, MainHero->GetHitSound, 0.6f);

            float NewDamage = CombatDamage;
            if (bHasEvolved)
                NewDamage = CombatDamage * DamageMultiplier;
            UGameplayStatics::ApplyDamage(MainHero, NewDamage, AIController, this, UDamageType::StaticClass());
        }
    }
}	

void AEnemyBase::ShieldCombatOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
    
}

void AEnemyBase::AggroOnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    if (OtherActor)
    {
        AMainHero* MainHero = Cast<AMainHero>(OtherActor);
        if (MainHero)
        {
            RotateTowardsTarget = true;
            MainHero->SetCombatTarget(this);
            MainHero->UpdateCombatTarget(); // make sure to always target the closest enemy
            HealthBarWidget->SetVisibility(true);
            SetState(State::CHASE_CLOSE);
        }
    }
}

void AEnemyBase::AggroOnOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{   
    if (OtherActor)
    {
        AMainHero* MainHero = Cast<AMainHero>(OtherActor);
        if (MainHero)
        {
            // Disengaging when out of range
            bHasValidTarget = false;
            MainHero->SetCombatTarget(nullptr);
            MainHero->UpdateCombatTarget();
            HealthBarWidget->SetVisibility(false);
            SetState(State::IDLE);
        }
    }
}

void AEnemyBase::ActivateCollision()
{
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}
	
void AEnemyBase::DeactivateCollision()
{
    CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemyBase::ActivateShieldCollision()
{
    ShieldCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
}
	
void AEnemyBase::DeactivateShieldCollision()
{
    ShieldCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemyBase::DeathEnd()
{
    GetMesh()->bPauseAnims = true;
    GetMesh()->bNoSkeletonUpdate = true;

    GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemyBase::Disappear, DeathDelay);
}

void AEnemyBase::Disappear()
{
    Destroy();
}

void AEnemyBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemyBase::Fire() // Called in AnimNotify
{
    //FVector SpawnLocation = FVector(0, 0, ProjectileSpawnLocation->GetComponentLocation().Z);
    AProjectile* Projectile = GetWorld()->SpawnActor<AProjectile>(
		ProjectileClass,
		ProjectileSpawnLocation->GetComponentLocation(),
		ProjectileSpawnLocation->GetComponentRotation() );
	    // projectileClass is a UClass object which is a blueprint based on C++ Projectile class
    if (Projectile == nullptr) return;
	Projectile->SetOwner(this); // this is to set the owner of the projectile to the pawn that spawned it.
}

void AEnemyBase::StartTaunt()
{
    GetCharacterMovement()->MaxWalkSpeed = 0;
    bEvolving = true;
}

void AEnemyBase::EndTaunt()
{
    GetCharacterMovement()->MaxWalkSpeed = 450.f;
    bEvolving = false;
}