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
	}
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (ShooterCharacter->IsLocallyControlled())
	{
		SetCrosshairsForWeapon(DeltaTime);
		FindCrosshairHitTarget(CrosshairHitResult);
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

	ShooterController = ShooterController == nullptr ? Cast<AShooterPlayerController>(ShooterCharacter->Controller) : ShooterController;

	if (ShooterController != nullptr)
	{
		ShooterHUD = ShooterHUD == nullptr ? Cast<AShooterHUD>(ShooterController->GetHUD()) : ShooterHUD;
		if (ShooterHUD)
		{
			FHUDPackage HUDPackage;
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

			HUDPackage.SpreadFactor = CrosshairSpreadFactor + CrosshairInAirFactor;
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

void UCombatComponent::SetFiringState(bool isFiring)
{
	bIsFireBtnPressed = isFiring;
	if (bIsFireBtnPressed)
	{
		FHitResult FireHitResult;
		FindCrosshairHitTarget(FireHitResult);
		ServerFire(FireHitResult.ImpactPoint);
	}
}

void UCombatComponent::ServerFire_Implementation(FVector_NetQuantize FireHitTarget)
{
	// Runs on server and clients for that particular actor
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
		GEngine->GameViewport->GetViewportSize(ViewportSize);

		FVector2D ViewportCenter(ViewportSize.X / 2, ViewportSize.Y / 2);
		FVector3d CrosshairWorldLocation, CrosshairWorldDirection;
		bool bIsDeprojected = UGameplayStatics::DeprojectScreenToWorld(
			UGameplayStatics::GetPlayerController(this, 0),
			ViewportCenter,
			CrosshairWorldLocation,
			CrosshairWorldDirection);

		if (bIsDeprojected)
		{
			FVector3d Start(CrosshairWorldLocation);
			FVector3d End(CrosshairWorldLocation + CrosshairWorldDirection * TRACE_LENGTH);
			GetWorld()->LineTraceSingleByChannel(
				HitResult,
				Start,
				End,
				ECollisionChannel::ECC_Visibility);
		}
	}
}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
}

void UCombatComponent::EquipWeapon(AWeapon *WeaponToEquip)
{
	if (!ShooterCharacter || !WeaponToEquip)
		return;

	EquippedWeapon = WeaponToEquip;

	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const USkeletalMeshSocket *RightHandSocket = ShooterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	RightHandSocket->AttachActor(EquippedWeapon, ShooterCharacter->GetMesh());

	EquippedWeapon->SetOwner(ShooterCharacter);
	// EquippedWeapon->ShowPickupWidget(false);
	ShooterCharacter->bUseControllerRotationYaw = true;
	ShooterCharacter->GetCharacterMovement()->bOrientRotationToMovement = false;
}

void UCombatComponent::ServerAimSync_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
}
