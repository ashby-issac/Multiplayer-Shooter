// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Pickup.h"
#include "HealthPickup.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AHealthPickup : public APickup
{
	GENERATED_BODY()

public:
	AHealthPickup();

protected:
	virtual void OnPickupAreaOverlap(UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult) override;

	virtual void Destroyed() override;

private:
	UPROPERTY(EditAnywhere)
	float HealthAmt = 100.f;

	UPROPERTY(EditAnywhere)
	float HealingTime = 5.f;

	UPROPERTY(EditAnywhere)
	class UNiagaraSystem* HealthPickupSystem;

	UPROPERTY(EditAnywhere)
	class UNiagaraComponent* HealthPickupComponent;
};
