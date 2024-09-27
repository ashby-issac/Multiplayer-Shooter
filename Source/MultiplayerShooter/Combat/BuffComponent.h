// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "BuffComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERSHOOTER_API UBuffComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UBuffComponent();
	friend class AShooterCharacter;

	void Heal(float HealthAmt, float HealTiming);
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);

private:	
	UPROPERTY()
	class AShooterCharacter* ShooterCharacter;
	
	bool bIsHealing = false;
	float HealthRate = 0.f;
	float HealAmt = 0.f;
	float HealthPerFrame = 0.f;
};
