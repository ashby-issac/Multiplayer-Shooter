// Fill out your copyright notice in the Description page of Project Settings.


#include "SpeedPickup.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "MultiplayerShooter/Combat/BuffComponent.h"

void ASpeedPickup::OnPickupAreaOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnPickupAreaOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter && ShooterCharacter->GetBuffComponent())
	{
		ShooterCharacter->GetBuffComponent()->BuffSpeed(BaseSpeed, CrouchSpeed, SpeedDuration);
	}

	Destroy();
}