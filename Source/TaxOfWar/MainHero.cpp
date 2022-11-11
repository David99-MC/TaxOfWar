// Fill out your copyright notice in the Description page of Project Settings.


#include "MainHero.h"

#include "GameFramework/Actor.h"
#include "Components/SphereComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/CapsuleComponent.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"
#include "Animation/AnimInstance.h"
#include "EnemyBase.h"
#include "Weapon.h"
#include "TwoHandSword.h"
#include "Sound/SoundCue.h"
#include "AttributeComponent.h"
#include "BowArrow.h"

// Sets default values
AMainHero::AMainHero()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	TargetLockDistance = 1500.0f;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	BaseTurnRate = 45.0f;
	BaseLookUpRate = 45.0f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.2f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("Camera Boom"));
	CameraBoom->SetupAttachment(RootComponent);
	
	// The camera follows at this distance behind the character
	CameraBoom->TargetArmLength = 500.0f;

	// Rotate the arm based on the controller
	CameraBoom->bUsePawnControlRotation = true;

	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));

	// Attach the camera to the end of the boom and let the boom adjust 
	// to match the controller orientation
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);

	// Camera does not rotate relative to arm
	FollowCamera->bUsePawnControlRotation = false;

    // doesn't need to set up attachment
    Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attribute Component"));

	Attacking = false;
	Rolling = false;
	TargetLocked = false;
	NextAttackReady = false;
	AttackDamaging = false;
	AttackIndex = 0;

	NormalSpeed = 500.f;
    RollSpeed = 650.f;
    StumblingSpeed = 40.f;
	GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;

    Stamina = MaxStamina;

	// StaminaRegenRate = 50.f;
    // SprintingDrainRate = 20.f;
    // RollStamina = 40.f;
	// MinSprintStamina = 50.f;
    // SprintingSpeed = 800.f;
    MovementStatus = EMovementStatus::EMS_Normal;
    StaminaStatus = EStaminaStatus::ESS_Normal;

    bShiftKeyDown = false;

    GetHitInterpSpeed = 10.f;
    InterpSpeed = 15.f;
    bInterpToEnemy = false;

    bMoveForward = false;
	bMoveRight = false;

}

// Called when the game starts or when spawned
void AMainHero::BeginPlay()
{
	Super::BeginPlay();

    WeaponType = EWeaponType::EWT_BareHand;

    AnimInstance = GetMesh()->GetAnimInstance();
}

// Called every frame
void AMainHero::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

    if (MovementStatus == EMovementStatus::EMS_Dead) return;
    
    // TODO: When in CombatMode
    FocusTarget();

    if (Rolling)
    {
        AddMovementInput(GetActorForwardVector(), RollSpeed * DeltaTime);
    }
    else if (Stumbling && MovingBackwards)
    {
        AddMovementInput(-GetActorForwardVector(), StumblingSpeed * GetWorld()->GetDeltaSeconds());
    }

    if (bInterpToEnemy && CombatTarget)
    {
        FRotator LookAtYaw = GetLookAtRotationYaw(CombatTarget->GetActorLocation());
        // Smoothly rotate towards the enemy
        FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, DeltaTime, InterpSpeed);
        SetActorRotation(InterpRotation);
    }

    Sprinting(DeltaTime);
}

void AMainHero::Sprinting(float DeltaSecond)
{
    float SprintDeltaStamina = SprintingDrainRate * DeltaSecond;
    float RegenDeltaStamina = StaminaRegenRate * DeltaSecond;

    switch (StaminaStatus)
    {
    case EStaminaStatus::ESS_Normal:
        if (bShiftKeyDown && (bMoveForward || bMoveRight))
        {
            SetMovementStatus(EMovementStatus::EMS_Sprinting);
            Stamina -= SprintDeltaStamina;
            if (Stamina <= StaminaWarningThreshold)
                SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
        }
        else // shift key up
        {
            Stamina = FMath::Min(Stamina + RegenDeltaStamina, MaxStamina);
            SetMovementStatus(EMovementStatus::EMS_Normal);
        }
        break;

    case EStaminaStatus::ESS_BelowMinimum:
        if (bShiftKeyDown && (bMoveForward || bMoveRight))
        {
            SetMovementStatus(EMovementStatus::EMS_Sprinting);
            Stamina -= SprintDeltaStamina;
            if (Stamina <= 0.f)
                SetStaminaStatus(EStaminaStatus::ESS_Exhausted);
        }
        else // shift key up
        {
            SetMovementStatus(EMovementStatus::EMS_Normal);
            Stamina += RegenDeltaStamina; 
            if (Stamina >= StaminaWarningThreshold)  
                SetStaminaStatus(EStaminaStatus::ESS_Normal);
        }
        break;

    case EStaminaStatus::ESS_Exhausted:
        SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovery);
		Stamina += RegenDeltaStamina;
		SetMovementStatus(EMovementStatus::EMS_Normal); // Change the MovementStatus and update movement speed
        break;

    case EStaminaStatus::ESS_ExhaustedRecovery:
        Stamina += RegenDeltaStamina; 
        if (Stamina >= StaminaWarningThreshold)  
            SetStaminaStatus(EStaminaStatus::ESS_Normal);
        SetMovementStatus(EMovementStatus::EMS_Normal);
        break;
    
    default:
        break;
    }
}

FRotator AMainHero::GetLookAtRotationYaw(FVector Enemy)
{
    // Find the rotation towards the enemy
    FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetActorLocation(), Enemy);
    FRotator LookAtRotationYaw(0, LookAtRotation.Yaw, 0);
    return LookAtRotationYaw;
}

// Called to bind functionality to input
void AMainHero::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	//Super::SetupPlayerInputComponent(PlayerInputComponent);
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMainHero::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMainHero::MoveRight);

	PlayerInputComponent->BindAxis("Turn", this, &APawn::AddControllerYawInput);
	PlayerInputComponent->BindAxis("LookUp", this, &APawn::AddControllerPitchInput);

	PlayerInputComponent->BindAction("Attack", IE_Pressed, this, &AMainHero::Attack);
	PlayerInputComponent->BindAction("Roll", IE_Pressed, this, &AMainHero::Roll);

    PlayerInputComponent->BindAction("Sprint", IE_Pressed, this, &AMainHero::ShiftKeyDown);
    PlayerInputComponent->BindAction("Sprint", IE_Released, this, &AMainHero::ShiftKeyUp);

    PlayerInputComponent->BindAction("EquipItem", IE_Pressed, this, &AMainHero::PickUpItem);

	PlayerInputComponent->BindAction("CombatModeToggle", IE_Pressed, this, &AMainHero::ToggleCombatMode);
}

void AMainHero::TurnAtRate(float Rate)
{
    AddControllerYawInput(Rate * BaseTurnRate * GetWorld()->GetDeltaSeconds());
}

void AMainHero::LookUpAtRate(float Rate)
{
    AddControllerPitchInput(Rate * BaseLookUpRate * GetWorld()->GetDeltaSeconds());
}

void AMainHero::MoveForward(float Value)
{
    // MoveForward and MoveRight are called EVERY frame
    // So we can initialize as false, when the button is pressed THEN we set it to true
    bMoveForward = false;
    if (Controller && Value != 0.f && !Attacking && !Rolling && !Stumbling && MovementStatus != EMovementStatus::EMS_Dead)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // Get the Direction based off of a rotation
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
        AddMovementInput(Direction, Value);
        bMoveForward = true;
    }
    InputDirection.X = Value;
}

void AMainHero::MoveRight(float Value)
{
    bMoveRight = false;
    if (Controller && Value != 0.f && !Attacking && !Rolling && !Stumbling && MovementStatus != EMovementStatus::EMS_Dead)
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // Get the Direction based off of a rotation
        const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
        AddMovementInput(Direction, Value);
        bMoveRight = true;
    }
    InputDirection.Y = Value;
}

void AMainHero::CycleTarget(bool Clockwise)
{

}

float AMainHero::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
    if (DamageCauser == this || Rolling)
        return 0.0;
    
    if (MovementStatus == EMovementStatus::EMS_Dead)
        return 0.0;
    
    // Turn to the enemy first then do the stumble animation
    EndAttack();
    FRotator LookAtYaw = GetLookAtRotationYaw(DamageCauser->GetActorLocation());
    // Smoothly rotate towards the enemy
    FRotator InterpRotation = FMath::RInterpTo(GetActorRotation(), LookAtYaw, GetWorld()->DeltaTimeSeconds, InterpSpeed * GetHitInterpSpeed);
    SetActorRotation(InterpRotation);
    
    SetMovingBackwards(false);
    SetMovingForward(false);
    Stumbling = true;

    if (AnimInstance)
    {
        switch (WeaponType)
        {
        case EWeaponType::EWT_BareHand:
            AnimInstance->Montage_Play(BareHandMontage);
            AnimInstance->Montage_JumpToSection("GetHitNW", BareHandMontage);
            break;
        case EWeaponType::EWT_TwoHandSword:
            AnimInstance->Montage_Play(TwoHandSwordMontage);
            AnimInstance->Montage_JumpToSection("GetHitTHS", TwoHandSwordMontage);
            break;
        case EWeaponType::EWT_Spear:
            AnimInstance->Montage_Play(SpearMontage);
            AnimInstance->Montage_JumpToSection("GetHitSpear", SpearMontage);
            break;
        case EWeaponType::EWT_BowArrow:
            AnimInstance->Montage_Play(BowArrowMontage);
            AnimInstance->Montage_JumpToSection("GetHitBow", BowArrowMontage);
            break;
        default:
            break;
        }
    }

    if (Attributes)
    {
        Attributes->ReceiveDamage(DamageAmount);
        if (Attributes->GetHealthPercent() <= 0)
        {
            ResetGame();
            if (DamageCauser)
            {
                AEnemyBase* Enemy = Cast<AEnemyBase>(DamageCauser);
                if (Enemy)
                    Enemy->bHasValidTarget = false;
            }
        }
    }
	return DamageAmount;
}

void AMainHero::Die()
{
    if (MovementStatus == EMovementStatus::EMS_Dead) return;
    SetMovementStatus(EMovementStatus::EMS_Dead);
    if (AnimInstance)
    {
        switch (WeaponType)
        {
        case EWeaponType::EWT_BareHand:
            AnimInstance->Montage_Play(BareHandMontage, 0.75f);
            AnimInstance->Montage_JumpToSection("DieNW", BareHandMontage);
            break;
        case EWeaponType::EWT_TwoHandSword:
            AnimInstance->Montage_Play(TwoHandSwordMontage, 0.75f);
            AnimInstance->Montage_JumpToSection("DieTHS", TwoHandSwordMontage);
            break;
        case EWeaponType::EWT_Spear:
            AnimInstance->Montage_Play(SpearMontage, 0.75f);
            AnimInstance->Montage_JumpToSection("DieSpear", SpearMontage);
            break;
        case EWeaponType::EWT_BowArrow:
            AnimInstance->Montage_Play(BowArrowMontage, 0.75f);
            AnimInstance->Montage_JumpToSection("DieBow", BowArrowMontage);
            break;
        default:
            break;
        }
    }
    // TODO: Set Game Over
}

void AMainHero::SetEquippedWeapon(AWeapon* WeaponToSet)
{
    if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
	EquippedWeapon = WeaponToSet;
}

void AMainHero::PickUpItem()
{
	if (ActiveOverlappingItem)
	{
		AWeapon* WeaponToPickUp = Cast<AWeapon>(ActiveOverlappingItem);
		WeaponToPickUp->Equip(this);
	}
}

void AMainHero::Attack()
{
    if ((!Attacking || NextAttackReady) && !Rolling && !Stumbling && !GetCharacterMovement()->IsFalling())
    {
        Super::Attack();
        if (Stamina < AttackStamina) return;
        SetInterpToEnemy(true);
        
        if (AttackIndex >= 4)
            AttackIndex = 0;

        // Play different AnimMontage based on weapon
        if (AnimInstance)
        {
            if (WeaponType == EWeaponType::EWT_TwoHandSword)
            {
                AnimInstance->Montage_Play(TwoHandSwordMontage, AttackSpeed);
                switch (AttackIndex++)
                {
                case 0:
                    AnimInstance->Montage_JumpToSection(FName("ComboTHS1"), TwoHandSwordMontage);
                    break;
                case 1:
                    AnimInstance->Montage_JumpToSection(FName("ComboTHS2"), TwoHandSwordMontage);
                    break;
                case 2:
                    AnimInstance->Montage_JumpToSection(FName("ComboTHS3"), TwoHandSwordMontage);
                    break;
                case 3:
                    AnimInstance->Montage_JumpToSection(FName("ComboTHS4"), TwoHandSwordMontage);
                    break;
                default:
                    break;
                }
            }
            else if (WeaponType == EWeaponType::EWT_BowArrow)
            {
                AnimInstance->Montage_Play(BowArrowMontage, AttackSpeed);
                switch (AttackIndex++)
                {
                case 0:
                    AnimInstance->Montage_JumpToSection(FName("ComboBow1"), BowArrowMontage);
                    break;
                case 1:
                    AnimInstance->Montage_JumpToSection(FName("ComboBow2"), BowArrowMontage);
                    break;
                case 2:
                    AnimInstance->Montage_JumpToSection(FName("ComboBow3"), BowArrowMontage);
                    break;
                case 3:
                    AnimInstance->Montage_JumpToSection(FName("ComboBow4"), BowArrowMontage);
                    break;
                default:
                    break;
                }
            }
            else if (WeaponType == EWeaponType::EWT_Spear)
            {
                AnimInstance->Montage_Play(SpearMontage, AttackSpeed);
                switch (AttackIndex++)
                {
                case 0:
                    AnimInstance->Montage_JumpToSection(FName("ComboSpear1"), SpearMontage);
                    break;
                case 1:
                    AnimInstance->Montage_JumpToSection(FName("ComboSpear2"), SpearMontage);
                    break;
                case 2:
                    AnimInstance->Montage_JumpToSection(FName("ComboSpear3"), SpearMontage);
                    break;
                case 3:
                    AnimInstance->Montage_JumpToSection(FName("ComboSpear4"), SpearMontage);
                    break;
                default:
                    break;
                }
            }
            else // Bare hand
            {
                AnimInstance->Montage_Play(BareHandMontage, 2.f);
                int RanInt = FMath::RandRange(0, 1);
                switch (RanInt)
                {
                case 0:
                    AnimInstance->Montage_JumpToSection(FName("ComboNW1"), BareHandMontage);
                    break;
                case 1:
                    AnimInstance->Montage_JumpToSection(FName("ComboNW2"), BareHandMontage);
                    break;
                default:
                    break;
                }
            }
            if (EquippedWeapon && EquippedWeapon->SwingSound)
                UGameplayStatics::PlaySound2D(this, EquippedWeapon->SwingSound);
        }
    }
}

void AMainHero::EndAttack()
{
    Super::EndAttack();
    bInterpToEnemy = false;
    AttackIndex = 0;
}

void AMainHero::UpdateCombatTarget() // Called when an enemy is killed or engaging with a new enemy or disengaged  
{
    TSet<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors, EnemyFilter);
    if (OverlappingActors.IsEmpty())
        return;

    float MinDistance = 99999;
    AActor* NextTarget = nullptr;
    for (auto& Actor : OverlappingActors)
    {
        float Distance = FVector::Distance(Actor->GetActorLocation(), GetActorLocation());
        if (Distance < MinDistance)
        {
            MinDistance = Distance;
            NextTarget = Actor;
        }
    }
    AEnemyBase* NextEnemy = Cast<AEnemyBase>(NextTarget);
    if (NextEnemy)
        SetCombatTarget(NextEnemy);
}

void AMainHero::Roll()
{
    if (Stamina >= RollStamina && MovementStatus != EMovementStatus::EMS_Dead)
    {
        if (Rolling || Stumbling)
        return;
    
        EndAttack();
        Rolling = true;
        if (InputDirection != FVector::ZeroVector)
        {
            FRotator PlayerRotZeroPitch = Controller->GetControlRotation();
            PlayerRotZeroPitch.Pitch = 0;

            FVector PlayerRight = FRotationMatrix(PlayerRotZeroPitch).GetUnitAxis(EAxis::Y);
            FVector PlayerForward = FRotationMatrix(PlayerRotZeroPitch).GetUnitAxis(EAxis::X);

            FVector DodgeDir = PlayerForward * InputDirection.X + PlayerRight * InputDirection.Y;

            RollRotation = DodgeDir.ToOrientationRotator();
        }
        else
            RollRotation = GetActorRotation();

        SetActorRotation(RollRotation);
        // Play different AnimMontage based on weapon
        if (AnimInstance)
        {
            switch (WeaponType)
            {
            case EWeaponType::EWT_TwoHandSword:
                AnimInstance->Montage_Play(TwoHandSwordMontage, 1.4f);
                AnimInstance->Montage_JumpToSection(FName("RollTHS"), TwoHandSwordMontage);
                break;
            case EWeaponType::EWT_Spear:
                AnimInstance->Montage_Play(SpearMontage, 1.4f);
                AnimInstance->Montage_JumpToSection(FName("RollSpear"), SpearMontage);
                break;
            case EWeaponType::EWT_BowArrow:
                AnimInstance->Montage_Play(BowArrowMontage, 1.4f);
                AnimInstance->Montage_JumpToSection(FName("RollBow"), BowArrowMontage);
                break;
            case EWeaponType::EWT_BareHand:
                AnimInstance->Montage_Play(BareHandMontage, 1.4f);
                AnimInstance->Montage_JumpToSection(FName("RollNW"), BareHandMontage);
                break;
            default:
                break;
            }
        }
    }
}

void AMainHero::StartRoll()
{
    Rolling = true;
    // Basically give the Player a boost in MovementSpeed when rolling
    GetCharacterMovement()->MaxWalkSpeed = RollSpeed;
    EndAttack();
}

void AMainHero::EndRoll()
{
    Rolling = false;
    GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
}

void AMainHero::SetAttackDamaging(bool Damaging)
{
    Super::SetAttackDamaging(Damaging);
}

void AMainHero::SetInterpToEnemy(bool Interp)
{
    bInterpToEnemy = Interp;
}

void AMainHero::FocusTarget()
{
    
}

void AMainHero::ToggleCombatMode()
{

}

void AMainHero::SetInCombat(bool InCombat)
{

}

void AMainHero::DeathEnd()
{
    GetMesh()->bPauseAnims = true;
    GetMesh()->bNoSkeletonUpdate = true;
}

void AMainHero::SetMovementStatus(EMovementStatus Status)
{
	MovementStatus = Status;
	if (MovementStatus == EMovementStatus::EMS_Sprinting)
	{
		GetCharacterMovement()->MaxWalkSpeed = SprintingSpeed;
	}
	else
	{
		GetCharacterMovement()->MaxWalkSpeed = NormalSpeed;
	}
}

void AMainHero::TakeStamina(float Amount)
{
    Stamina -= Amount;
    if (Stamina <= StaminaWarningThreshold)
        SetStaminaStatus(EStaminaStatus::ESS_BelowMinimum);
    else if(Stamina <= 0)
        SetStaminaStatus(EStaminaStatus::ESS_ExhaustedRecovery);
}

bool AMainHero::GetRolling()
{
    return Rolling;
}

void AMainHero::ShiftKeyUp()
{
    bShiftKeyDown = false;
}

void AMainHero::ShiftKeyDown()
{
    bShiftKeyDown = true;
}

void AMainHero::Fire()
{
    ABowArrow* Bow = Cast<ABowArrow>(EquippedWeapon);
    if (Bow)
        Bow->ShootArrow();
    
}