// Fill out your copyright notice in the Description page of Project Settings.


#include "HealthPickup.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/Combat/BuffComponent.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"

AHealthPickup::AHealthPickup()
{

}

void AHealthPickup::OnPickupAreaOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnPickupAreaOverlap(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);
	
	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(OtherActor);
	if (ShooterCharacter)
	{
		ShooterCharacter->GetBuffComponent()->Heal(HealthAmt, HealingTime);
	}

	Destroy();
}