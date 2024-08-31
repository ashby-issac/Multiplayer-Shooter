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

/************************************ PUBLIC FUNCTIONS ************************************/

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

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
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

void UCombatComponent::SetFiringState(bool isFiring)
{
	bIsFireBtnPressed = isFiring;
	if (bIsFireBtnPressed)
	{
		Fire();
	}
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

void UCombatComponent::OnReloadFinished()
{
	if (ShooterCharacter == nullptr)
		return;

	if (ShooterCharacter->HasAuthority())
	{
		CombatState = ECombatState::ECS_Unoccupied;
		CalculateReloading();
	}

	if (bIsFireBtnPressed)
	{
		UE_LOG(LogTemp, Warning, TEXT("UCombatComponent::OnReloadFinished"));
		Fire();
	}
}

/************************************ PROTECTED FUNCTIONS ************************************/

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

void UCombatComponent::OnRep_CombatState()
{
	switch (CombatState)
	{
	case ECombatState::ECS_Reloading:
		HandleReload();
		break;
	case ECombatState::ECS_Unoccupied:
		if (bIsFireBtnPressed)
		{
			Fire();
		}
		break;
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	UpdateCarriedAmmoHUD();
}

void UCombatComponent::ServerFire_Implementation(FVector_NetQuantize FireHitTarget)
{
	MulticastFire(FireHitTarget);
}

void UCombatComponent::ServerAimSync_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}

void UCombatComponent::ServerReload_Implementation()
{
	if (ShooterCharacter == nullptr)
		return;

	CombatState = ECombatState::ECS_Reloading;
	HandleReload();
}

void UCombatComponent::MulticastFire_Implementation(FVector_NetQuantize FireHitTarget)
{
	if (!ShooterCharacter || !EquippedWeapon)
		return;

	if (CombatState == ECombatState::ECS_Unoccupied)
	{
		EquippedWeapon->Fire(FireHitTarget);
		ShooterCharacter->PlayFireMontage(bAiming);
	}
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

void UCombatComponent::HandleReload()
{
	if (ShooterCharacter == nullptr || EquippedWeapon == nullptr)
		return;

	ShooterCharacter->PlayReloadMontage();
}

void UCombatComponent::CalculateReloading()
{
	int32 MagCapacity = EquippedWeapon->GetWeaponMagCapacity();
	int32 WeaponAmmo = EquippedWeapon->GetAvailableAmmo();
	int32 ReloadAmt = MagCapacity - WeaponAmmo;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		UE_LOG(LogTemp, Warning, TEXT("CarriedAmmo: %d"), CarriedAmmo);
		EquippedWeapon->UpdateAmmoData(FMath::Min(ReloadAmt, CarriedAmmo));
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] = ReloadAmt <= CarriedAmmo ? 
															CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmt : 0;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	EquippedWeapon->UpdateWeaponAmmoHUD();
	UpdateCarriedAmmoHUD();
}

/************************************ PRIVATE FUNCTIONS ************************************/

void UCombatComponent::ReloadWeapon()
{
	if (CarriedAmmo > 0 && CombatState != ECombatState::ECS_Reloading)
	{
		ServerReload();
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
		Fire();
	}
}

void UCombatComponent::Fire()
{
	if (!CanFire())
	{
		return;
	}

	ServerFire(CrosshairHitTarget);
	if (EquippedWeapon->bIsAutomaticWeapon) // for auto mode
	{
		EnableAutomaticFiring();
	}
	else
	{
		// WaitAndFire for weapons like ShotGun
	}
}

bool UCombatComponent::CanFire()
{
	return EquippedWeapon != nullptr && EquippedWeapon->GetAvailableAmmo() > 0 && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Add(EWeaponType::EWT_AssaultRifle, ARInitialAmmo);
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