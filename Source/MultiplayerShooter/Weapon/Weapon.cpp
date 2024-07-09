
#include "Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/WidgetComponent.h"
#include "Net/UnrealNetwork.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	// makes weapon a replicating actor so server is responsible for controlling all weapon objects
	bReplicates = true;

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	EquipArea = CreateDefaultSubobject<USphereComponent>(TEXT("EquipArea"));
	EquipArea->SetupAttachment(RootComponent);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EquipArea->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	EquipArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);

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
		EquipArea->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		EquipArea->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
		EquipArea->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnEquipAreaOverlap);
		EquipArea->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnEquipAreaEndOverlap);
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
		break;
	}
}

void AWeapon::SetWeaponState(EWeaponState CurrentState)
{
	WeaponState = CurrentState;
	switch (WeaponState)
	{
	case EWeaponState::EWS_Equipped:
		EquipArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		break;
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AWeapon::GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AWeapon, WeaponState);
}

void AWeapon::ShowPickupWidget(bool isEnabled)
{
	if (PickupWidget)
	{
		PickupWidget->SetVisibility(isEnabled);
	}
}

void AWeapon::Fire(const FVector& HitLocation)
{
	if (FireAnimation && WeaponMesh)
	{
		// Play SFX and VFX
		WeaponMesh->PlayAnimation(FireAnimation, false);
	}
}