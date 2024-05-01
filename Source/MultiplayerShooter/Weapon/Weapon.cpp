
#include "Weapon.h"
#include "Components/SphereComponent.h"

AWeapon::AWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	// making the weapon a replicating actor so server is responsible for all weapon objects
	bReplicates = true; 

	WeaponMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("WeaponMesh"));
	WeaponMesh->SetupAttachment(RootComponent);
	SetRootComponent(WeaponMesh);

	WeaponMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
	WeaponMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	WeaponMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EquipArea = CreateDefaultSubobject<USphereComponent>(TEXT("EquipArea"));
	EquipArea->SetupAttachment(RootComponent);

	EquipArea->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	EquipArea->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		EquipArea->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
		EquipArea->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}

void AWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

