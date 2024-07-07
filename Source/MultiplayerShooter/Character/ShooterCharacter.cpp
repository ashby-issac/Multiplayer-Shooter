// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerShooter/Weapon/Weapon.h"
#include "MultiplayerShooter/Combat/CombatComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/SkeletalMeshComponent.h"
#include "Animation/AnimInstance.h"

AShooterCharacter::AShooterCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArm"));
	SpringArmComponent->SetupAttachment(GetMesh());
	SpringArmComponent->TargetArmLength = 600.f;
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;

	bUseControllerRotationYaw = false;
	// rotate the character according to the mouseX,
	// if set to true the controller's rotation gets overridden
	// and character won't be oriented.
	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	TurnInPlaceState = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 720.f);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CalculateAimOffsets(DeltaTime);
}

void AShooterCharacter::CalculateAimOffsets(float DeltaTime)
{
	if (!IsWeaponEquipped())
	{
		return;
	}

	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		FRotator CurrentAimRot = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		/*	InitialAimRot: Initial Rotation from place before rotating
			CurrentAimRot: BaseAimRotation (Rotation relative to the world's rotation)
		*/
		// UE_LOG(LogTemp, Warning, TEXT(":: InitialAimRot: %s"), *InitialAimRot.ToString());
		// UE_LOG(LogTemp, Warning, TEXT(":: CurrentAimRot: %s"), *CurrentAimRot.ToString());

		DeltaAimRot = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRot, InitialAimRot);
		AO_Yaw = DeltaAimRot.Yaw; // 0 - 89... 90

		if (TurnInPlaceState == ETurningInPlace::ETIP_NotTurning)
		{
			Interp_AO = AO_Yaw; // -89 || -90 degrees
		}

		bUseControllerRotationYaw = true;
		// Set Turning In Place states
		CheckForTurningInPlace(DeltaTime);
	}
	else if (Speed > 0.f || bIsInAir)
	{
		TurnInPlaceState = ETurningInPlace::ETIP_NotTurning;
		InitialAimRot = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		bUseControllerRotationYaw = true;
		AO_Yaw = 0.f;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;

	if (!IsLocallyControlled() && AO_Pitch > 90.f)
	{
		FVector2D InRange = FVector2D(270.f, 360.f);
		FVector2D OutRange = FVector2D(-90.f, 0.f);

		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AShooterCharacter::CheckForTurningInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		// Changes animation state
		TurnInPlaceState = ETurningInPlace::ETIP_TurnRight;
	}
	else if (AO_Yaw < -90.f)
	{
		TurnInPlaceState = ETurningInPlace::ETIP_TurnLeft;
	}

	if (TurnInPlaceState != ETurningInPlace::ETIP_NotTurning) // if any of Turning states
	{
		// Setting AO_Yaw from InterpAO to 0.f means value will be
		// reflected in blend space and as a result the upper body
		// animation would  be layered on top of the turn left/right animation
		Interp_AO = FMath::FInterpTo(Interp_AO, 0.f, DeltaTime, 4.f);
		AO_Yaw = Interp_AO;

		if (FMath::Abs(AO_Yaw) < 10.f)
		{
			// Resetting state will make the animation
			// go back to idle state inside equipped state machine
			TurnInPlaceState = ETurningInPlace::ETIP_NotTurning;
			InitialAimRot = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}

	// UE_LOG(LogTemp, Warning, TEXT(":: ShooterCharacter => AO_Yaw: %f"), AO_Yaw);
	// UE_LOG(LogTemp, Warning, TEXT(":: ShooterCharacter => ActorRotation: %s"), *GetActorRotation().ToString());
	// UE_LOG(LogTemp, Warning, TEXT(":: ShooterCharacter => ControlRotation: %s"), *Controller->GetControlRotation().ToString());
}

void AShooterCharacter::PlayFireMontage(bool bAiming)
{
	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireMontage)
	{
		AnimInstance->Montage_Play(FireMontage);
		FName FireSection = bAiming ? FName("Aim_Fire") : FName("Hip_Fire");
		AnimInstance->Montage_JumpToSection(FireSection, FireMontage);
		UE_LOG(LogTemp, Warning, TEXT(":: OnFire PlayMontage"));
	}
}

void AShooterCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0)
	{
		FRotator Rotation(0, Controller->GetControlRotation().Yaw, 0);
		FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
		Direction.Z = 0.f; // zeroing vertical axis

		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0)
	{
		FRotator Rotation(0, Controller->GetControlRotation().Yaw, 0);
		FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
		Direction.Z = 0.f; // zeroing vertical axis

		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::LookUp(float Value)
{
	AddControllerPitchInput(Value);
}

void AShooterCharacter::LookRight(float Value)
{
	AddControllerYawInput(Value);
}

void AShooterCharacter::EquipWeapon()
{
	if (CombatComponent && HasAuthority()) // if on server
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
		UE_LOG(LogTemp, Warning, TEXT("Has Authority"));
	}
	else
	{
		// Not on server
		/* Execute RPC function on the server machine
		   by calling it from the client machine */
		ServerEquipButtonPressed();
	}
}

void AShooterCharacter::OnAimPressed()
{
	if (CombatComponent)
	{
		CombatComponent->SetAimingState(true);
	}
}

void AShooterCharacter::OnAimReleased()
{
	if (CombatComponent)
	{
		CombatComponent->SetAimingState(false);
	}
}

void AShooterCharacter::OnFirePressed()
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon)
	{
		return;
	}

	CombatComponent->SetFiringState(true);
}

void AShooterCharacter::OnFireReleased()
{
	if (!CombatComponent || !CombatComponent->EquippedWeapon)
	{
		return;
	}

	CombatComponent->SetFiringState(false);
}

void AShooterCharacter::OnCrouchPressed()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	Crouch();
}

void AShooterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent)
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(FName("Jump"), EInputEvent::IE_Pressed, this, &ThisClass::Jump);
	PlayerInputComponent->BindAction(FName("EquipWeapon"), EInputEvent::IE_Pressed, this, &ThisClass::EquipWeapon);
	PlayerInputComponent->BindAction(FName("Crouch"), EInputEvent::IE_Pressed, this, &ThisClass::OnCrouchPressed);
	PlayerInputComponent->BindAction(FName("Aim"), EInputEvent::IE_Pressed, this, &ThisClass::OnAimPressed);
	PlayerInputComponent->BindAction(FName("Aim"), EInputEvent::IE_Released, this, &ThisClass::OnAimReleased);
	PlayerInputComponent->BindAction(FName("Fire"), EInputEvent::IE_Pressed, this, &ThisClass::OnFirePressed);
	PlayerInputComponent->BindAction(FName("Fire"), EInputEvent::IE_Released, this, &ThisClass::OnFireReleased);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &AShooterCharacter::LookUp);
	PlayerInputComponent->BindAxis(FName("LookRight"), this, &AShooterCharacter::LookRight);
}

void AShooterCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch();
	}
	else
	{
		Super::Jump();
	}
}

void AShooterCharacter::OnRep_OverlappingWeapon(AWeapon *LastWeapon)
{
	UE_LOG(LogTemp, Warning, TEXT(":: OnRep_OverlappingWeapon"));
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void AShooterCharacter::SetOverlappingWeapon(AWeapon *Weapon)
{
	if (OverlappingWeapon)
	{
		// disabling the widget for the server when not overlapping with the weapon
		OverlappingWeapon->ShowPickupWidget(false);
	}

	// OnRep_OverlappingWeapon gets called after value's set if not server
	OverlappingWeapon = Weapon;

	if (IsLocallyControlled()) // if server's character
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true);
		}
	}
}

bool AShooterCharacter::IsWeaponEquipped()
{
	return CombatComponent && CombatComponent->EquippedWeapon;
}

bool AShooterCharacter::IsAiming()
{
	return CombatComponent && CombatComponent->bAiming;
}

AWeapon *AShooterCharacter::GetEquippedWeapon()
{
	return !CombatComponent ? nullptr : CombatComponent->GetEquippedWeapon();
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->ShooterCharacter = this;
	}
}
