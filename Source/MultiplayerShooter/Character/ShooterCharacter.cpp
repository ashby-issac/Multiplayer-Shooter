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
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CalculateAimOffsets();
}

void AShooterCharacter::CalculateAimOffsets()
{
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (Speed == 0.f && !bIsInAir)
	{
		FRotator CurrentAimRot = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		DeltaAimRot = UKismetMathLibrary::NormalizedDeltaRotator(CurrentAimRot, InitialAimRot);
		AO_Yaw = DeltaAimRot.Yaw;
		bUseControllerRotationYaw = false;
	}

	if (Speed > 0.f || bIsInAir)
	{
		InitialAimRot = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		AO_Yaw = 0.f;
		bUseControllerRotationYaw = true;
	}

	AO_Pitch = GetBaseAimRotation().Pitch;

	if (!IsLocallyControlled() && AO_Pitch > 90.f)
	{
		FVector2D InRange = FVector2D(270.f, 360.f);
		FVector2D OutRange = FVector2D(-90.f, 0.f);

		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AShooterCharacter::MoveForward(float Value)
{
	if (Controller && Value != 0)
	{
		FRotator Rotation(0, Controller->GetControlRotation().Yaw, 0);
		FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::X);
		Direction.Z = 0.f;
		
		AddMovementInput(Direction, Value);
	}
}

void AShooterCharacter::MoveRight(float Value)
{
	if (Controller && Value != 0)
	{
		FRotator Rotation(0, Controller->GetControlRotation().Yaw, 0);
		FVector Direction = FRotationMatrix(Rotation).GetUnitAxis(EAxis::Y);
		Direction.Z = 0.f;
		
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
	if (CombatComponent && HasAuthority())
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
	}
	else
	{
		//  Call RPC function on server
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

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(FName("Jump"), EInputEvent::IE_Pressed, this, &ThisClass::OnJump);
	PlayerInputComponent->BindAction(FName("EquipWeapon"), EInputEvent::IE_Pressed, this, &ThisClass::EquipWeapon);
	PlayerInputComponent->BindAction(FName("Crouch"), EInputEvent::IE_Pressed, this, &ThisClass::OnCrouchPressed);
	PlayerInputComponent->BindAction(FName("Aim"), EInputEvent::IE_Pressed, this, &ThisClass::OnAimPressed);
	PlayerInputComponent->BindAction(FName("Aim"), EInputEvent::IE_Released, this, &ThisClass::OnAimReleased);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &AShooterCharacter::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &AShooterCharacter::MoveRight);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &AShooterCharacter::LookUp);
	PlayerInputComponent->BindAxis(FName("LookRight"), this, &AShooterCharacter::LookRight);
}

void AShooterCharacter::OnJump()
{
	UE_LOG(LogTemp, Warning, TEXT(":: Rotation: %s"), *GetActorRotation().ToString());
	SetActorRotation(GetActorRotation().ZeroRotator);
	Jump();
}

void AShooterCharacter::OnRep_OverlappingWeapon(AWeapon* LastWeapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(true);
	}

	if (LastWeapon)
	{
		LastWeapon->ShowPickupWidget(false);
	}
}

void AShooterCharacter::SetOverlappingWeapon(AWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		// disabling the widget for the server when not overlapping with the weapon
		OverlappingWeapon->ShowPickupWidget(false);
	}

	OverlappingWeapon = Weapon;
	if (IsLocallyControlled()) // if character controlled by server itself
	{
		if (OverlappingWeapon)
		{
			OverlappingWeapon->ShowPickupWidget(true); // OwnerOnly
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

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
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





