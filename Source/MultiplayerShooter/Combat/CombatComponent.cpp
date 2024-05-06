#include "CombatComponent.h"
#include "MultiplayerShooter/Weapon/Weapon.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Net/UnrealNetwork.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

}


void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
}

void UCombatComponent::EquipWeapon(AWeapon* WeaponToEquip)
{
	if (!ShooterCharacter || !WeaponToEquip) return;

	EquippedWeapon = WeaponToEquip;
	
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);
	
	const USkeletalMeshSocket* RightHandSocket = ShooterCharacter->GetMesh()->GetSocketByName(FName("RightHandSocket"));
	RightHandSocket->AttachActor(EquippedWeapon, ShooterCharacter->GetMesh());

	EquippedWeapon->SetOwner(ShooterCharacter);
	//EquippedWeapon->ShowPickupWidget(false);
	
}

