// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "ShieldPickup.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API AShieldPickup : public APickup
{
	GENERATED_BODY()

protected:
	virtual void OnPickupAreaOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

private:
	UPROPERTY(EditAnywhere)
	float ShieldReplenishAmt = 100.f;

	UPROPERTY(EditAnywhere)
	float SheildReplenishTime = 5.f;
};
