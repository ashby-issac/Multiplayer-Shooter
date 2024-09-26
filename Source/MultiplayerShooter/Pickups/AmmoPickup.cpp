// Fill out your copyright notice in the Description page of Project Settings.


#include "AmmoPickup.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "MultiplayerShooter/Combat/CombatComponent.h"

void AAmmoPickup::BeginPlay()
{
	Super::BeginPlay();

}

void AAmmoPickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (PickupMesh)
	{
		PickupMesh->AddWorldRotation(FRotator(0.f, TurnRotationSpeed * DeltaTime, 0.f));
	}
}

void AAmmoPickup::OnPickupAreaOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnPickupAreaOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		UCombatComponent* Combat = ShooterCharacter->GetCombatComponent();
		if (Combat)
		{
			Combat->AddPickedupAmmo(WeaponType, AmmoAmt);
		}
	}
	Destroy();
}

void AAmmoPickup::Destroyed()
{
	Super::Destroyed();
}