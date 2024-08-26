#include "CombatComponent.h"
#include "MultiplayerShooter/Weapon/Weapon.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "MultiplayerShooter/PlayerController/ShooterPlayerController.h"
#include "MultiplayerShooter/HUD/ShooterHUD.h"
#include "Camera/CameraComponent.h"
#include "MultiplayerShooter/Interfaces/CrosshairsInteractor.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (ShooterCharacter)
	{
		ShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
		if (ShooterCharacter->GetFollowCam())
		{
			DefaultZoomFOV = ShooterCharacter->GetFollowCam()->FieldOfView;
			CurrentFOV = DefaultZoomFOV;
		}
		InitializeCarriedAmmo();
	}
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Add(EWeaponType::EWT_AssaultRifle, ARInitialAmmo);
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ShooterCharacter->IsLocallyControlled())
	{
		FindCrosshairHitTarget(CrosshairHitResult);
		SetCrosshairsForWeapon(DeltaTime);
		CrosshairHitTarget = CrosshairHitResult.ImpactPoint;
		SetZoomedFOV(DeltaTime);
	}
}

void UCombatComponent::SetZoomedFOV(float DeltaTime)
{
	if (EquippedWeapon == nullptr)
	{
		return;
	}

	if (bAiming)
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, EquippedWeapon->GetZoomedFOV(), DeltaTime,
									  EquippedWeapon->GetZoomedInterpSpeed());
	}
	else
	{
		CurrentFOV = FMath::FInterpTo(CurrentFOV, DefaultZoomFOV, DeltaTime,
									  DefaultZoomInterpSpeed);
	}

	if (ShooterCharacter && ShooterCharacter->GetFollowCam())
	{
		ShooterCharacter->GetFollowCam()->SetFieldOfView(CurrentFOV);
	}
}

void UCombatComponent::SetCrosshairsForWeapon(float DeltaTime)
{
	if (ShooterCharacter == nullptr || ShooterCharacter->Controller == nullptr)
	{
		return;
	}

	ShooterController = ShooterController == nullptr ? Cast<AShooterPlayerController>(ShooterCharacter->Controller)
													 : ShooterController;

	if (ShooterController != nullptr)
	{
		ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(ShooterController->GetHUD()) : ShooterHUD;
		if (ShooterHUD)
		{
			if (EquippedWeapon)
			{
				HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairCenter;
				HUDPackage.CrosshairTop = EquippedWeapon->CrosshairTop;
				HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairBottom;
				HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairLeft;
				HUDPackage.CrosshairRight = EquippedWeapon->CrosshairRight;
			}
			else
			{
				HUDPackage.CrosshairCenter = nullptr;
				HUDPackage.CrosshairTop = nullptr;
				HUDPackage.CrosshairBottom = nullptr;
				HUDPackage.CrosshairLeft = nullptr;
				HUDPackage.CrosshairRight = nullptr;
			}

			FVector2D WalkSpeed(0.f, ShooterCharacter->GetCharacterMovement()->MaxWalkSpeed);
			FVector2D ClampedSpeed(0.f, 1.f);

			FVector CharacterVelocity = ShooterCharacter->GetVelocity();
			CharacterVelocity.Z = 0.f;

			float CharacterSpeed = CharacterVelocity.Size();
			float CrosshairSpreadFactor = FMath::GetMappedRangeValueClamped(WalkSpeed, ClampedSpeed, CharacterSpeed);

			CrosshairInAirFactor = ShooterCharacter->GetCharacterMovement()->IsFalling() ? FMath::FInterpTo(CrosshairInAirFactor, 2.f, DeltaTime, 2.f) : FMath::FInterpTo(CrosshairInAirFactor, 0.f, DeltaTime, 30.f);

			CrosshairAimFactor = bAiming ? FMath::FInterpTo(CrosshairAimFactor, 0.5f, DeltaTime, 30.f)
										 : FMath::FInterpTo(CrosshairAimFactor, 0.f, DeltaTime, 30.f);

			HUDPackage.SpreadFactor = CrosshairSpreadFactor + CrosshairInAirFactor - CrosshairAimFactor;
			ShooterHUD->SetHUDPackageProps(HUDPackage);
		}
	}
}

void UCombatComponent::OnRep_OnEquippedWeapon()
{
	if (!ShooterCharacter || !EquippedWeapon)
	{
		return;
	}

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket *RightHandSocket = ShooterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket != nullptr)
	{
		RightHandSocket->AttachActor(EquippedWeapon, ShooterCharacter->GetMesh());
	}

	ShooterCharacter->bUseControllerRotationYaw = true;
	ShooterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
}

void UCombatComponent::SetAimingState(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (ShooterCharacter)
	{
		ShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}

	ServerAimSync(bIsAiming);
}

void UCombatComponent::EnableAutomaticFiring()
{
	bCanFire = false;
	ShooterCharacter->GetWorldTimerManager().SetTimer(FireRateTimerHandle,
													  this,
													  &ThisClass::OnFireDelayed,
													  EquippedWeapon->FireDelay,
													  false);
}

void UCombatComponent::OnFireDelayed()
{
	bCanFire = true;
	if (bIsFireBtnPressed) // if button is still on hold
	{
		SetFiringState(true);
	}
}

void UCombatComponent::Fire()
{
	ServerFire(CrosshairHitTarget);
}

void UCombatComponent::SetFiringState(bool isFiring)
{
	if (!CanFire())
	{
		return;
	}

	bIsFireBtnPressed = isFiring;
	if (bIsFireBtnPressed && bCanFire)
	{
		Fire();
		if (EquippedWeapon->bIsAutomaticWeapon) // feature for auto mode
		{
			EnableAutomaticFiring();
		}
		else
		{
			// WaitAndFire for weapons like ShotGun
		}
	}
}

bool UCombatComponent::CanFire()
{
	return EquippedWeapon != nullptr && EquippedWeapon->GetWeaponAmmo() > 0;
}

void UCombatComponent::ServerFire_Implementation(FVector_NetQuantize FireHitTarget)
{
	MulticastFire(FireHitTarget);
}

void UCombatComponent::MulticastFire_Implementation(FVector_NetQuantize FireHitTarget)
{
	if (!ShooterCharacter || !EquippedWeapon)
		return;

	EquippedWeapon->Fire(FireHitTarget);
	ShooterCharacter->PlayFireMontage(bAiming);
}

void UCombatComponent::FindCrosshairHitTarget(FHitResult &HitResult)
{
	if (GEngine && GEngine->GameViewport)
	{
		FVector2D ViewportSize;
		if (GEngine && GEngine->GameViewport)
		{
			GEngine->GameViewport->GetViewportSize(ViewportSize);
		}

		FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
		FVector CrosshairWorldLocation, CrosshairWorldDirection;
		bool bIsDeprojected = UGameplayStatics::DeprojectScreenToWorld(
			UGameplayStatics::GetPlayerController(this, 0),
			ViewportCenter,
			CrosshairWorldLocation,
			CrosshairWorldDirection);

		if (bIsDeprojected)
		{
			FVector Start(CrosshairWorldLocation);

			float Distance = (ShooterCharacter->GetActorLocation() - Start).Size();
			Start += CrosshairWorldDirection * (Distance + 100.f);

			FVector3d End(Start + CrosshairWorldDirection * TRACE_LENGTH);

			GetWorld()->LineTraceSingleByChannel(
				HitResult,
				Start,
				End,
				ECollisionChannel::ECC_Visibility);

			if (HitResult.GetActor() != nullptr && HitResult.GetActor()->Implements<UCrosshairsInteractor>())
			{
				// DrawDebugSphere(GetWorld(), Start, 30.f, 12, FColor::Red);
				HUDPackage.CrosshairColor = FLinearColor::Red;
			}
			else
			{
				// DrawDebugSphere(GetWorld(), Start, 30.f, 12, FColor::Black);
				HUDPackage.CrosshairColor = FLinearColor::Black;
			}
		}
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	UpdateCarriedAmmoHUD();
}

void UCombatComponent::UpdateCarriedAmmoHUD()
{
	ShooterController = ShooterController == nullptr ? Cast<AShooterPlayerController>(ShooterCharacter->Controller) : ShooterController;
	if (ShooterController != nullptr)
	{
		if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
		{
			ShooterController->SendCarriedAmmoHUDUpdate(CarriedAmmo);
		}
	}
}

void UCombatComponent::EquipWeapon(AWeapon *WeaponToEquip)
{
	if (!ShooterCharacter || !WeaponToEquip)
		return;

	if (EquippedWeapon != nullptr)
	{
		EquippedWeapon->Dropped();
	}

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket *RightHandSocket = ShooterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket != nullptr)
	{
		RightHandSocket->AttachActor(EquippedWeapon, ShooterCharacter->GetMesh());
	}
	
	EquippedWeapon->SetOwner(ShooterCharacter);

	CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	EquippedWeapon->UpdateWeaponAmmoHUD();
	UpdateCarriedAmmoHUD();

	ShooterCharacter->bUseControllerRotationYaw = true;
	ShooterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
}

void UCombatComponent::ServerAimSync_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}
