// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterCharacter.h"
#include "TimerManager.h"
#include "Sound/SoundCue.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Animation/AnimInstance.h"
#include "Particles/ParticleSystem.h"
#include "Kismet/KismetMathLibrary.h"
#include "Components/WidgetComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "MultiplayerShooter/Weapon/Weapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "MultiplayerShooter/Weapon/WeaponTypes.h"
#include "MultiplayerShooter/Combat/BuffComponent.h"
#include "MultiplayerShooter/HUD/CharacterOverlay.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MultiplayerShooter/Combat/CombatComponent.h"
#include "MultiplayerShooter/GameModes/ShooterGameMode.h"
#include "MultiplayerShooter/PlayerStates/ShooterPlayerState.h"
#include "MultiplayerShooter/PlayerController/ShooterPlayerController.h"

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

	BuffComponent = CreateDefaultSubobject<UBuffComponent>(TEXT("BuffComponent"));
	BuffComponent->SetIsReplicated(true);

	DissolveTimeline = CreateDefaultSubobject<UTimelineComponent>(TEXT("DissolveTimelineComponent"));
	GrenadeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("GrenadeMesh"));
	GrenadeMesh->SetupAttachment(GetMesh(), FName("GrenadeSocket"));
	GrenadeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;

	TurnInPlaceState = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;

	GetCharacterMovement()->RotationRate = FRotator(0.f, 0.f, 720.f);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (IsLocallyControlled())
	{
		Health = MaxHealth;
		UpdatePlayerHealthHUD();
	}

	if (HasAuthority())
		OnTakeAnyDamage.AddDynamic(this, &ThisClass::ReceiveDamage);

	if (GrenadeMesh)
	{
		GrenadeMesh->SetVisibility(false);
	}
}

void AShooterCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	RotateInPlace(DeltaTime);
	CheckCamIsOnCloseContact();
	PollInit();
}

void AShooterCharacter::RotateInPlace(float DeltaTime)
{
	if (bDisableGameplay)
	{
		TurnInPlaceState = ETurningInPlace::ETIP_NotTurning;
		bUseControllerRotationYaw = false; // prevent character rotation
		return;
	}

	if (GetLocalRole() > ENetRole::ROLE_SimulatedProxy && IsLocallyControlled())
	{
		CalculateAimOffsets(DeltaTime);
	}
	else
	{
		TimeForProxyRotation += DeltaTime;
		if (TimeForProxyRotation > 0.25f)
			OnRep_ReplicatedMovement();

		CalculatePitch();
	}
}

void AShooterCharacter::PollInit()
{
	if (ShooterPlayerState == nullptr && IsLocallyControlled())
	{
		ShooterPlayerState = GetPlayerState<AShooterPlayerState>();
		if (ShooterPlayerState)
		{
			ShooterPlayerState->AddToScore(0.f);
			ShooterPlayerState->AddToDefeats(0.f);
		}
	}
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
		bRotateRootBone = true;
		FRotator CurrentAimRot = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		/*	InitialAimRot: Initial Rotation from place before rotating
			CurrentAimRot: BaseAimRotation (Rotation relative to the world's rotation) */

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
		bRotateRootBone = false;
		TurnInPlaceState = ETurningInPlace::ETIP_NotTurning;
		InitialAimRot = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		bUseControllerRotationYaw = true;
		AO_Yaw = 0.f;
	}

	CalculatePitch();
}

void AShooterCharacter::CalculatePitch()
{
	AO_Pitch = GetBaseAimRotation().Pitch;
	if (!IsLocallyControlled() && AO_Pitch > 90.f)
	{
		FVector2D InRange = FVector2D(270.f, 360.f);
		FVector2D OutRange = FVector2D(-90.f, 0.f);

		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AShooterCharacter::SimulatedProxiesTurn()
{
	if (!CombatComponent && !CombatComponent->EquippedWeapon)
	{
		return;
	}

	bRotateRootBone = false;
	ProxyRotationLastFrame = CurrentProxyRotation;
	CurrentProxyRotation = GetActorRotation();
	ProxyRotYaw = UKismetMathLibrary::NormalizedDeltaRotator(ProxyRotationLastFrame, CurrentProxyRotation).Yaw;

	if (FMath::Abs(ProxyRotYaw) > TurnThreshold)
	{
		if (ProxyRotYaw > TurnThreshold)
		{
			TurnInPlaceState = ETurningInPlace::ETIP_TurnRight;
		}
		else if (ProxyRotYaw < -TurnThreshold)
		{
			TurnInPlaceState = ETurningInPlace::ETIP_TurnLeft;
		}
		else
		{
			TurnInPlaceState = ETurningInPlace::ETIP_NotTurning;
		}
		return;
	}

	TurnInPlaceState = ETurningInPlace::ETIP_NotTurning;
}

void AShooterCharacter::OnRep_ReplicatedMovement()
{
	Super::OnRep_ReplicatedMovement();

	SimulatedProxiesTurn();
	TimeForProxyRotation = 0.f;
}

void AShooterCharacter::CheckCamIsOnCloseContact()
{
	if (!IsLocallyControlled())
		return;

	float DistanceToCam = (GetActorLocation() - CameraComponent->GetComponentLocation()).Size();
	if (DistanceToCam < CamThreshold)
		DisableMeshesOnCloseContact(false);
	else
		DisableMeshesOnCloseContact(true);
}

void AShooterCharacter::DisableMeshesOnCloseContact(bool bVisible)
{
	GetMesh()->SetVisibility(bVisible);
	if (CombatComponent && CombatComponent->EquippedWeapon &&
		CombatComponent->EquippedWeapon->GetWeaponMesh())
	{
		CombatComponent->EquippedWeapon->GetWeaponMesh()->bOwnerNoSee = !bVisible;
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
}

void AShooterCharacter::PlayFireMontage(bool bAiming)
{
	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance != nullptr && FireWeaponMontage != nullptr)
	{
		AnimInstance->Montage_Play(FireWeaponMontage);
		FName FireSection = bAiming ? FName("Aim_Fire") : FName("Hip_Fire");
		AnimInstance->Montage_JumpToSection(FireSection, FireWeaponMontage);
	}
}

void AShooterCharacter::PlayReloadMontage()
{
	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance != nullptr && ReloadMontage != nullptr)
	{
		AnimInstance->Montage_Play(ReloadMontage);
		FName ReloadSection;

		switch (CombatComponent->EquippedWeapon->GetWeaponType())
		{
		case EWeaponType::EWT_AssaultRifle:
			ReloadSection = "Reload_Rifle";
			break;
		case EWeaponType::EWT_RocketLaucher:
			ReloadSection = "RocketLauncher";
			break;
		case EWeaponType::EWT_Pistol:
			ReloadSection = "Pistol";
			break;
		case EWeaponType::EWT_SMG:
			ReloadSection = "Pistol";
			break;
		case EWeaponType::EWT_Shotgun:
			ReloadSection = "Shotgun";
			break;
		case EWeaponType::EWT_SniperRifle:
			ReloadSection = "SniperRifle";
			break;
		case EWeaponType::EWT_GrenadeLauncher:
			ReloadSection = "GrenadeLauncher";
			break;
		}

		AnimInstance->Montage_JumpToSection(ReloadSection, ReloadMontage);
	}
}

void AShooterCharacter::PlayHitReactMontage()
{
	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		FName FireSection("HitFront");
		AnimInstance->Montage_JumpToSection(FireSection, HitReactMontage);
	}
}

void AShooterCharacter::PlayElimMontage()
{
	UAnimInstance *AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ElimMontage)
		AnimInstance->Montage_Play(ElimMontage);
}

void AShooterCharacter::PlayGrenadeThrowMontage()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && GrenadeThrowMontage)
	{
		AnimInstance->Montage_Play(GrenadeThrowMontage);
	}
}

void AShooterCharacter::MoveForward(float Value)
{
	if (bDisableGameplay) return;

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
	if (bDisableGameplay) return;

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
	if (bDisableGameplay) return;

	if (CombatComponent && HasAuthority()) // if on server
	{
		CombatComponent->EquipWeapon(OverlappingWeapon);
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
	if (bDisableGameplay) return;

	if (CombatComponent)
	{
		CombatComponent->SetAimingState(true);
	}
}

void AShooterCharacter::OnAimReleased()
{
	if (bDisableGameplay || CombatComponent)
	{
		CombatComponent->SetAimingState(false);
	}
}

void AShooterCharacter::OnFirePressed()
{
	if (bDisableGameplay || !CombatComponent || !CombatComponent->EquippedWeapon)
		return;

	CombatComponent->SetFiringState(true);
}

void AShooterCharacter::OnFireReleased()
{
	if (bDisableGameplay || !CombatComponent || !CombatComponent->EquippedWeapon)
		return;

	CombatComponent->SetFiringState(false);
}

void AShooterCharacter::OnGrenadeThrowPressed()
{
	if (!CombatComponent) return;

	CombatComponent->PlayGrenadeThrowAction();
}

void AShooterCharacter::OnCrouchPressed()
{
	if (bDisableGameplay) return;

	if (bIsCrouched)
		UnCrouch();

	Crouch();
}

void AShooterCharacter::OnReloadPressed()
{
	if (bDisableGameplay) return;

	if (CombatComponent != nullptr)
		CombatComponent->ReloadWeapon();
}

void AShooterCharacter::ServerEquipButtonPressed_Implementation()
{
	if (CombatComponent != nullptr)
		CombatComponent->EquipWeapon(OverlappingWeapon);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent *PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAction(FName("Jump"), EInputEvent::IE_Pressed, this, &ThisClass::Jump);
	PlayerInputComponent->BindAction(FName("EquipWeapon"), EInputEvent::IE_Pressed, this, &ThisClass::EquipWeapon);
	PlayerInputComponent->BindAction(FName("Crouch"), EInputEvent::IE_Pressed, this, &ThisClass::OnCrouchPressed);
	PlayerInputComponent->BindAction(FName("Reload"), EInputEvent::IE_Pressed, this, &ThisClass::OnReloadPressed);

	PlayerInputComponent->BindAction(FName("Aim"), EInputEvent::IE_Pressed, this, &ThisClass::OnAimPressed);
	PlayerInputComponent->BindAction(FName("Aim"), EInputEvent::IE_Released, this, &ThisClass::OnAimReleased);

	PlayerInputComponent->BindAction(FName("Fire"), EInputEvent::IE_Pressed, this, &ThisClass::OnFirePressed);
	PlayerInputComponent->BindAction(FName("Fire"), EInputEvent::IE_Released, this, &ThisClass::OnFireReleased);
	PlayerInputComponent->BindAction(FName("GrenadeThrow"), EInputEvent::IE_Pressed, this, &ThisClass::OnGrenadeThrowPressed);

	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &ThisClass::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &ThisClass::MoveRight);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &ThisClass::LookUp);
	PlayerInputComponent->BindAxis(FName("LookRight"), this, &ThisClass::LookRight);
}

void AShooterCharacter::Jump()
{
	if (bDisableGameplay) return;

	FName Auth = HasAuthority() ? FName("HasAuth") : FName("HasNoAuth");

	if (bIsCrouched)
		UnCrouch();
	else
		Super::Jump();
}

void AShooterCharacter::OnRep_OverlappingWeapon(AWeapon *LastWeapon)
{
	if (OverlappingWeapon)
		OverlappingWeapon->ShowPickupWidget(true);

	if (LastWeapon)
		LastWeapon->ShowPickupWidget(false);
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

FVector AShooterCharacter::GetCrosshairHitTarget()
{
	return CombatComponent ? CombatComponent->CrosshairHitTarget : FVector();
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AShooterCharacter, OverlappingWeapon, COND_OwnerOnly);
	DOREPLIFETIME(AShooterCharacter, Health); // TODO :: TEST
}

void AShooterCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
		CombatComponent->ShooterCharacter = this;

	if (BuffComponent)
	{
		BuffComponent->ShooterCharacter = this;
		if (GetCharacterMovement())
		{
			BuffComponent->SetInitialSpeeds(GetCharacterMovement()->MaxWalkSpeed, GetCharacterMovement()->MaxWalkSpeedCrouched);
		}
	}
}

void AShooterCharacter::ReceiveDamage(AActor *DamagedActor, float Damage, const UDamageType *DamageType, AController *InstigatedBy, AActor *DamageCauser)
{
	Health = FMath::Clamp(Health - Damage, 0.f, MaxHealth);

	UpdatePlayerHealthHUD();
	PlayHitReactMontage();

	if (Health <= 0.f)
	{
		AShooterGameMode *ShooterGameMode = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode());
		AShooterPlayerController *InsigatorController = Cast<AShooterPlayerController>(InstigatedBy);

		ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
		if (ShooterGameMode != nullptr)
		{
			ShooterGameMode->OnPlayerEliminated(this, ShooterPlayerController, InsigatorController);
		}
	}
}

void AShooterCharacter::OnEliminated()
{
	if (CombatComponent != nullptr && CombatComponent->EquippedWeapon != nullptr)
		CombatComponent->EquippedWeapon->Dropped();

	MulticastEliminate();
	GetWorldTimerManager().SetTimer(ElimTimerHandle, this, &ThisClass::RespawnCharacter, ElimDelay);
}

void AShooterCharacter::RespawnCharacter()
{
	AShooterGameMode *ShooterGameMode = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode());
	if (ShooterGameMode != nullptr)
	{
		ShooterGameMode->RespawnPlayer(Controller, this);
	}
}

void AShooterCharacter::MulticastEliminate_Implementation()
{
	bIsEliminated = true;
	PlayElimMontage();

	if (ShooterPlayerController != nullptr)
	{
		ShooterPlayerController->SendWeaponAmmoHUDUpdate(0.f);
	}

	if (DissolveMaterialInstance != nullptr)
	{
		DynamicDissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicDissolveMaterialInstance);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("DissolveAmt"), -0.55f);
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("Glow"), 200.f);
	}

	StartDissolve(); // Start Dissolving the Character On Elim

	GetCharacterMovement()->DisableMovement();
	GetCharacterMovement()->StopMovementImmediately();

	bDisableGameplay = true;
	GetCharacterMovement()->DisableMovement();
	if (CombatComponent != nullptr)
	{
		CombatComponent->SetFiringState(false);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	if (ElimBotParticle != nullptr)
	{
		ElimBotComponent = UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			ElimBotParticle,
			FVector(GetActorLocation().X, GetActorLocation().Y, GetActorLocation().Z + ElimBotZOffset),
			GetActorRotation());
	}

	if (ElimBotSoundCue != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(GetWorld(), ElimBotSoundCue, GetActorLocation());
	}
}

void AShooterCharacter::Destroyed()
{
	Super::Destroyed();

	if (ElimBotComponent != nullptr)
		ElimBotComponent->DestroyComponent();

	AShooterGameMode* ShooterGameMode = Cast<AShooterGameMode>(UGameplayStatics::GetGameMode(this));
    bool bInCooldownState =	CombatComponent != nullptr && CombatComponent->EquippedWeapon != nullptr
		&& ShooterGameMode != nullptr && ShooterGameMode->GetMatchState() != MatchState::InProgress;
	if (bInCooldownState)
		CombatComponent->EquippedWeapon->Destroy();
}

void AShooterCharacter::OnRep_HealthDamaged(float PrevHealth)
{
	UpdatePlayerHealthHUD();
	if (PrevHealth > Health)
		PlayHitReactMontage();
}

void AShooterCharacter::UpdatePlayerHealthHUD()
{
	ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(Controller) : ShooterPlayerController;
	if (ShooterPlayerController != nullptr)
	{
		ShooterPlayerController->SendHealthHUDUpdate(Health, MaxHealth);
	}
}

void AShooterCharacter::StartDissolve()
{
	DissolveTrack.BindDynamic(this, &ThisClass::UpdateDissolveMaterial);
	if (DissolveTimeline && DissolveCurve)
	{
		DissolveTimeline->AddInterpFloat(DissolveCurve, DissolveTrack);
		DissolveTimeline->Play();
	}
}

void AShooterCharacter::UpdateDissolveMaterial(float DissolveValue)
{
	if (DynamicDissolveMaterialInstance != nullptr)
	{
		DynamicDissolveMaterialInstance->SetScalarParameterValue(TEXT("DissolveAmt"), DissolveValue);
	}
}

ECombatState AShooterCharacter::GetCombatState() const
{
	if (CombatComponent == nullptr) return ECombatState::ETIP_MAX;

	return CombatComponent->CombatState;
} 
