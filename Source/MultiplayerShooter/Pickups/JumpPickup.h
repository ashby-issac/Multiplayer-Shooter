// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "JumpPickup.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API AJumpPickup : public APickup
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
	float JumpZVelocity = 2000.f;

	UPROPERTY(EditAnywhere)
	float JumpVelocityDuration = 5.f;

};
