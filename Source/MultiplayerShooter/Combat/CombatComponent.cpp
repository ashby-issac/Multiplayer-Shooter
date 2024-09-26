#include "CombatComponent.h"
#include "Sound/SoundCue.h"
#include "DrawDebugHelpers.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "Camera/CameraComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/SkeletalMeshSocket.h"
#include "MultiplayerShooter/Weapon/Weapon.h"
#include "MultiplayerShooter/HUD/ShooterHUD.h"
#include "MultiplayerShooter/Weapon/WeaponTypes.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MultiplayerShooter/Weapon/ProjectileGrenade.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "MultiplayerShooter/Character/ShooterAnimInstance.h"
#include "MultiplayerShooter/Interfaces/CrosshairsInteractor.h"
#include "MultiplayerShooter/PlayerController/ShooterPlayerController.h"

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
		SetZoomedFOV(DeltaTime);
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, CombatState);
	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME_CONDITION(UCombatComponent, CarriedAmmo, COND_OwnerOnly);
	DOREPLIFETIME(UCombatComponent, Grenades);
}

void UCombatComponent::DropEquippedWeapon()
{
	if (EquippedWeapon != nullptr)
	{
		EquippedWeapon->Dropped();
	}
}

void UCombatComponent::AttachToLeftHand(AActor* ActorToAttach)
{
	if (ActorToAttach == nullptr || ShooterCharacter == nullptr || ShooterCharacter->GetMesh() == nullptr
		|| EquippedWeapon == nullptr) return;

	FName SocketName = FName("LeftHandSocket");
	bool bUsePistol = EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Pistol 
						|| EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SMG;
	if (bUsePistol)
		SocketName = FName("PistolSocket");

	const USkeletalMeshSocket* LeftHandSocket = ShooterCharacter->GetMesh()->GetSocketByName(SocketName);
	if (LeftHandSocket != nullptr)
	{
		LeftHandSocket->AttachActor(ActorToAttach, ShooterCharacter->GetMesh());
	}
}

void UCombatComponent::AttachToRightHand(AActor* ActorToAttach)
{
	if (ActorToAttach == nullptr || ShooterCharacter == nullptr || ShooterCharacter->GetMesh() == nullptr) return;

	const USkeletalMeshSocket* RightHandSocket = ShooterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	if (RightHandSocket != nullptr)
	{
		RightHandSocket->AttachActor(ActorToAttach, ShooterCharacter->GetMesh());
	}
}

void UCombatComponent::PlayWeaponEquipSFX()
{
	if (ShooterCharacter == nullptr || EquippedWeapon == nullptr || EquippedWeapon->EquipSound == nullptr) return;

	UGameplayStatics::PlaySoundAtLocation(
		this,
		EquippedWeapon->EquipSound,
		ShooterCharacter->GetActorLocation());
}

void UCombatComponent::EquipWeapon(AWeapon *WeaponToEquip)
{
	if (!ShooterCharacter || !WeaponToEquip || CombatState != ECombatState::ECS_Unoccupied)
		return;

	DropEquippedWeapon();

	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachToRightHand(EquippedWeapon);
	EquippedWeapon->SetOwner(ShooterCharacter);

	UpdateCarriedAmmoData();
	EquippedWeapon->UpdateWeaponAmmoHUD();
	PlayWeaponEquipSFX();

	ShooterCharacter->bUseControllerRotationYaw = true;
	ShooterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
}

void UCombatComponent::ReloadEmptyWeapon()
{
	if (EquippedWeapon->GetAvailableAmmo() < 1)
	{
		ReloadWeapon();
	}
}

void UCombatComponent::SetGrenadeActiveState(bool bEnable)
{
	if (ShooterCharacter && ShooterCharacter->GetGrenadeMesh())
	{
		ShooterCharacter->GetGrenadeMesh()->SetVisibility(bEnable);
	}
}

void UCombatComponent::SetFiringState(bool isFiring)
{
	bIsFireBtnPressed = isFiring;
	if (bIsFireBtnPressed)
		Fire();
}

void UCombatComponent::SetAimingState(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (ShooterCharacter != nullptr)
	{
		ShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	
		if (EquippedWeapon != nullptr && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_SniperRifle)
		{
			ShooterCharacter->ShowSniperScope(bIsAiming);
		}
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
		Fire();
	}
}

void UCombatComponent::OnShellInserted()
{
	if (ShooterCharacter != nullptr && ShooterCharacter->HasAuthority())
		CalculateReloadPerInsert();
}

void UCombatComponent::OnGrenadeThrowFinished()
{
	CombatState = ECombatState::ECS_Unoccupied;
	AttachToRightHand(EquippedWeapon);
}

void UCombatComponent::LaunchGrenade()
{
	SetGrenadeActiveState(false);
	if (ShooterCharacter && ShooterCharacter->IsLocallyControlled())
	{
		ServerLaunchGrenade(CrosshairHitTarget);
	}
}

/************************************ PROTECTED FUNCTIONS ************************************/

void UCombatComponent::ServerLaunchGrenade_Implementation(FVector_NetQuantize HitTarget)
{
	if (ShooterCharacter && GrenadeClass && ShooterCharacter->GetGrenadeMesh())
	{
		FVector GrenadeLocation = ShooterCharacter->GetGrenadeMesh()->GetComponentLocation();
		FVector TargetLocation = HitTarget - GrenadeLocation;

		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = ShooterCharacter;
		SpawnParams.Instigator = ShooterCharacter;

		if (auto World = GetWorld())
		{
			World->SpawnActor<AProjectileAmmo>(
				GrenadeClass,
				GrenadeLocation,
				TargetLocation.Rotation(),
				SpawnParams);
		}
	}
}

void UCombatComponent::OnRep_OnEquippedWeapon()
{
	if (ShooterCharacter == nullptr || EquippedWeapon == nullptr)
		return;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	AttachToRightHand(EquippedWeapon);
	PlayWeaponEquipSFX();

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
	case ECombatState::ECS_GrenadeThrow:
		if (ShooterCharacter && !ShooterCharacter->IsLocallyControlled())
		{
			AttachToLeftHand(EquippedWeapon);
			SetGrenadeActiveState(true);
			ShooterCharacter->PlayGrenadeThrowMontage();
		}
		break;
	}
}

void UCombatComponent::OnRep_CarriedAmmo()
{
	UpdateCarriedAmmoData();
	bool bJumpToShotgunEnd = EquippedWeapon && EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun && CombatState == ECombatState::ECS_Reloading && CarriedAmmo == 0;
	if (bJumpToShotgunEnd)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::OnRep_Grenades()
{
	UpdateGrenadesData();
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

void UCombatComponent::ServerGrenadeThrow_Implementation()
{
	CombatState = ECombatState::ECS_GrenadeThrow;
	if (ShooterCharacter)
	{
		AttachToLeftHand(EquippedWeapon);
		SetGrenadeActiveState(true);
		ShooterCharacter->PlayGrenadeThrowMontage();
	}

	Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
	UpdateGrenadesData();

}

void UCombatComponent::MulticastFire_Implementation(FVector_NetQuantize FireHitTarget)
{
	if (ShooterCharacter == nullptr || EquippedWeapon == nullptr)
		return;

	bool bIsShotgunReloading = EquippedWeapon != nullptr && EquippedWeapon->GetAvailableAmmo() > 0 &&
		CombatState == ECombatState::ECS_Reloading &&
		EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun;
	if (bIsShotgunReloading)
	{
		EquippedWeapon->Fire(FireHitTarget);
		ShooterCharacter->PlayFireMontage(bAiming);
		CombatState = ECombatState::ECS_Unoccupied;
		return;
	}

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

			CrosshairHitTarget = HitResult.GetActor() != nullptr ? HitResult.ImpactPoint : End;

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
	if (ShooterCharacter == nullptr || EquippedWeapon == nullptr || EquippedWeapon->IsFull())
		return;

	int32 MagCapacity = EquippedWeapon->GetWeaponMagCapacity();
	int32 WeaponAmmo = EquippedWeapon->GetAvailableAmmo();
	int32 ReloadAmt = MagCapacity - WeaponAmmo;

	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
	{
		EquippedWeapon->UpdateAmmoData(FMath::Min(ReloadAmt, CarriedAmmo));
		CarriedAmmoMap[EquippedWeapon->GetWeaponType()] = ReloadAmt <= CarriedAmmo ? CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmt : 0;
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	EquippedWeapon->UpdateWeaponAmmoHUD();
	UpdateCarriedAmmoData();
}

void UCombatComponent::CalculateReloadPerInsert()
{
	if (ShooterCharacter == nullptr || EquippedWeapon == nullptr)
		return;

	int32 ReloadAmt = 1;
	if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()) && !EquippedWeapon->IsFull())
	{
		if (ReloadAmt <= CarriedAmmo)
		{
			EquippedWeapon->UpdateAmmoData(ReloadAmt);
			CarriedAmmoMap[EquippedWeapon->GetWeaponType()] -= ReloadAmt;
		}
		else
		{
			CarriedAmmoMap[EquippedWeapon->GetWeaponType()] = 0;
		}

		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];
	}

	bCanFire = true;
	UpdateCarriedAmmoData();
	EquippedWeapon->UpdateWeaponAmmoHUD();
	if (EquippedWeapon->IsFull() || CarriedAmmo == 0)
	{
		JumpToShotgunEnd();
	}
}

void UCombatComponent::JumpToShotgunEnd()
{
	if (ShooterCharacter == nullptr) return;

	UAnimInstance* AnimInstance = ShooterCharacter->GetMesh()->GetAnimInstance();
	if (AnimInstance != nullptr && ShooterCharacter->GetReloadMontage() != nullptr)
	{
		AnimInstance->Montage_JumpToSection(FName("ShotgunEnd"));
	}
}

void UCombatComponent::PlayGrenadeThrowAction()
{
	if (CombatState == ECombatState::ECS_GrenadeThrow || EquippedWeapon == nullptr || Grenades < 1)
		return;

	CombatState = ECombatState::ECS_GrenadeThrow;
	if (ShooterCharacter)
	{
		AttachToLeftHand(EquippedWeapon);
		SetGrenadeActiveState(true);
		ShooterCharacter->PlayGrenadeThrowMontage();
		if (!ShooterCharacter->HasAuthority())
		{
			ServerGrenadeThrow();
		}
		else
		{
			Grenades = FMath::Clamp(Grenades - 1, 0, MaxGrenades);
			UpdateGrenadesData();
		}
	}
}

void UCombatComponent::AddPickedupAmmo(EWeaponType WeaponType, int32 AmmoAmt)
{
	EWeaponType EquippedWeaponType = EquippedWeapon ? EquippedWeapon->GetWeaponType() : EWeaponType::EWT_MAX;
	if (CarriedAmmoMap.Contains(WeaponType))
	{
		CarriedAmmoMap[WeaponType] = FMath::Clamp(CarriedAmmoMap[WeaponType] + AmmoAmt, 0, MaxCarriedAmmo);
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Black,
			FString::Printf(TEXT("Update CarriedAmmoMap"))
		);
		if (EquippedWeapon && WeaponType == EquippedWeaponType)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Black,
				FString::Printf(TEXT("UpdateCarriedAmmoData"))
			);
			UpdateCarriedAmmoData();
		}
	}

	if (EquippedWeapon && EquippedWeaponType == WeaponType && EquippedWeapon->GetAvailableAmmo() < 1)
	{
		ReloadWeapon();
	}
}

/************************************ PRIVATE FUNCTIONS ************************************/

void UCombatComponent::ReloadWeapon()
{
	if (CarriedAmmo > 0 && CombatState == ECombatState::ECS_Unoccupied)
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
		if (ShooterHUD != nullptr)
		{
			if (EquippedWeapon != nullptr)
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

void UCombatComponent::EnableFiringEvent()
{
	bCanFire = false;
	if (EquippedWeapon->GetAvailableAmmo() < 1)
	{
		ReloadWeapon();
	}
	ShooterCharacter->GetWorldTimerManager().SetTimer(FireRateTimerHandle,
													  this,
													  &ThisClass::OnFireDelayed,
													  EquippedWeapon->FireDelay,
													  false);
}

void UCombatComponent::OnFireDelayed()
{
	bCanFire = true;
	if (bIsFireBtnPressed && EquippedWeapon->bIsAutomaticWeapon) // if button is still on hold
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
	EnableFiringEvent();
}

bool UCombatComponent::CanFire()
{
	bool bIsShotgunReloading = CombatState == ECombatState::ECS_Reloading &&
								EquippedWeapon != nullptr && 
								EquippedWeapon->GetAvailableAmmo() > 0 &&
								EquippedWeapon->GetWeaponType() == EWeaponType::EWT_Shotgun;
	if (bIsShotgunReloading)
		return true;

	return EquippedWeapon != nullptr && EquippedWeapon->GetAvailableAmmo() > 0 && bCanFire && CombatState == ECombatState::ECS_Unoccupied;
}

void UCombatComponent::InitializeCarriedAmmo()
{
	CarriedAmmoMap.Add(EWeaponType::EWT_AssaultRifle, InitialRifleAmmo);
	CarriedAmmoMap.Add(EWeaponType::EWT_RocketLaucher, InitialRocketAmmo);
	CarriedAmmoMap.Add(EWeaponType::EWT_Pistol, InitialPistolAmmo);
	CarriedAmmoMap.Add(EWeaponType::EWT_SMG, InitialSMGAmmo);
	CarriedAmmoMap.Add(EWeaponType::EWT_Shotgun, InitialShotgunAmmo);
	CarriedAmmoMap.Add(EWeaponType::EWT_SniperRifle, InitialSniperAmmo);
	CarriedAmmoMap.Add(EWeaponType::EWT_GrenadeLauncher, InitialGrenadeLauncherAmmo);
}

void UCombatComponent::UpdateCarriedAmmoData()
{
	if (ShooterCharacter->HasAuthority())
		CarriedAmmo = CarriedAmmoMap[EquippedWeapon->GetWeaponType()];

	ShooterController = ShooterController == nullptr ? Cast<AShooterPlayerController>(ShooterCharacter->Controller) : ShooterController;
	if (ShooterController != nullptr)
	{
		if (CarriedAmmoMap.Contains(EquippedWeapon->GetWeaponType()))
		{
			ShooterController->SendCarriedAmmoHUDUpdate(CarriedAmmo);
		}
	}
}

void UCombatComponent::UpdateGrenadesData()
{
	ShooterController = ShooterController == nullptr ? Cast<AShooterPlayerController>(ShooterCharacter->Controller) : ShooterController;
	if (ShooterController != nullptr)
	{
		ShooterController->SendGrenadesHUDUpdate(Grenades);
	}
}