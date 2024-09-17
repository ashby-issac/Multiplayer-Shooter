
#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "MultiplayerShooter/PlayerController/ShooterPlayerController.h"
#include "Engine/SkeletalMeshSocket.h"
#include "AmmoShell.h"
#include "Sound/SoundCue.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	// makes weapon a replicating actor so server is responsible for controlling all weapon objects
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	EquipAreaSphere = CreateDefaultSubobject<USphereComponent>(TEXT("EquipArea"));
	EquipAreaSphere->SetupAttachment(RootComponent);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EquipAreaSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	EquipAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	PickupWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("PickupText"));
	PickupWidget->SetupAttachment(RootComponent);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (PickupWidget)
	{
		PickupWidget->SetVisibility(false);
	}

	if (HasAuthority())
	{
		EquipAreaSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		EquipAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		EquipAreaSphere->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnEquipAreaOverlap);
		EquipAreaSphere->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEquipAreaEndOverlap);
	}
}

void AWeapon::OnEquipAreaOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	AShooterCharacter *ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		ShooterCharacter->SetOverlappingWeapon(this);
	}
}

void AWeapon::OnEquipAreaEndOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex)
{
	AShooterCharacter *ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		ShooterCharacter->SetOverlappingWeapon(nullptr);
	}
}

void AWeapon::OnRep_WeaponState()
{
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		ShowPickupWidget(false); // disabled pickup widget on all other clients
		if (WeaponType == EWeaponType::EWT_SMG)
		{
			SetCollisionProps(ECollisionEnabled::QueryAndPhysics, false, true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
			return;
		}
		SetCollisionProps(ECollisionEnabled::NoCollision, false, false);
		break;
	case EWeaponState::EWS_Dropped:
		SetCollisionProps(ECollisionEnabled::QueryAndPhysics, true, true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		break;
	}
}

void AWeapon::OnRep_Owner()
{
	Super::OnRep_Owner();

	if (GetOwner() == nullptr)
	{
		PlayerCharacter = nullptr;
		ShooterPlayerController = nullptr;
	}
	else
	{
		UpdateWeaponAmmoHUD();
	}
}

void AWeapon::UpdateWeaponAmmoRound(int32 AmmoCount)
{
	Ammo -= AmmoCount;

	UpdateWeaponAmmoHUD();
}

void AWeapon::UpdateWeaponAmmoHUD()
{
	PlayerCharacter = PlayerCharacter == nullptr ? Cast<AShooterCharacter>(GetOwner()) : PlayerCharacter;
	if (PlayerCharacter)
	{
		ShooterPlayerController = ShooterPlayerController == nullptr ? Cast<AShooterPlayerController>(PlayerCharacter->Controller)
																	 : ShooterPlayerController;
		if (ShooterPlayerController)
		{
			ShooterPlayerController->SendWeaponAmmoHUDUpdate(Ammo);
		}
	}
}

void AWeapon::OnRep_Ammo()
{
	UpdateWeaponAmmoHUD();
}

void AWeapon::SetWeaponState(EWeaponState CurrentState)
{
	WeaponState = CurrentState;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		if (HasAuthority())
		{
			EquipAreaSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		}
		if (WeaponType == EWeaponType::EWT_SMG)
		{
			WeaponMesh->SetSimulatePhysics(false);
			SetCollisionProps(ECollisionEnabled::QueryAndPhysics, false, true);
			WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore); // Strap doesn't collide with anything
			return;
		}
		SetCollisionProps(ECollisionEnabled::NoCollision, false, false);
		break;
	case EWeaponState::EWS_Dropped:
		if (HasAuthority())
		{
			EquipAreaSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
		}
		SetCollisionProps(ECollisionEnabled::QueryAndPhysics, true, true);
		WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
		WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
		break;
	}
}

void AWeapon::SetCollisionProps(ECollisionEnabled::Type CollisionState, bool bSimulatePhysics, bool bEnableGravity)
{
	WeaponMesh->SetCollisionEnabled(CollisionState);
	WeaponMesh->SetSimulatePhysics(bSimulatePhysics);
	WeaponMesh->SetEnableGravity(bEnableGravity);
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
	DOREPLIFETIME(AWeapon, Ammo);
}

void AWeapon::ShowPickupWidget(bool isEnabled)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(isEnabled);
	}
}

void AWeapon::Fire(const FVector &HitLocation)
{
	if (FireAnimation != nullptr && WeaponMesh != nullptr)
	{
		// Play MuzzleFlash at tip of the Gun along with the sound
		WeaponMesh->PlayAnimation(FireAnimation, false);

		const USkeletalMeshSocket *AmmoEjectSocket = WeaponMesh->GetSocketByName(FName("AmmoEject"));

		if (AmmoEjectSocket != nullptr)
		{
			FTransform AmmoEjectTransform = AmmoEjectSocket->GetSocketTransform(WeaponMesh);
			FRotator DefaultRotation = AmmoEjectTransform.GetRotation().Rotator();
			
			if (AmmoShellClass != nullptr)
			{
				AAmmoShell* ShellInstance = GetWorld()->SpawnActor<AAmmoShell>(
					AmmoShellClass,
					AmmoEjectTransform.GetLocation(),
					DefaultRotation);
				if (ShellInstance != nullptr)
				{
					FRotator RandRotation = FRotator(FMath::RandRange(0.f, 180.f),
						FMath::RandRange(0.f, 180.f),
						FMath::RandRange(0.f, 180.f));
					ShellInstance->SetActorRotation(RandRotation);
				}
			}
		}
	}
	UpdateWeaponAmmoRound(1);
}

void AWeapon::Dropped()
{
	FDetachmentTransformRules DetachRules(EDetachmentRule::KeepWorld, true);
	WeaponMesh->DetachFromComponent(DetachRules);
	SetWeaponState(EWeaponState::EWS_Dropped);
	SetOwner(nullptr);
	PlayerCharacter = nullptr;
	ShooterPlayerController = nullptr;
}

void AWeapon::UpdateAmmoData(int32 Ammos)
{
	Ammo += Ammos;
}