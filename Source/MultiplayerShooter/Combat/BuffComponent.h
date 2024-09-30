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
	void BuffSpeed(float BaseSpeed, float CrouchSpeed, float SpeedDuration);
	void BuffJump(float JumpZVelocity, float JumpVelocityDuration);

	void SetInitialSpeeds(float BaseSpeed, float CrouchSpeed);
	void SetInitialJumpZVel(float JumpZVel);

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

protected:
	virtual void BeginPlay() override;

	void HealRampUp(float DeltaTime);
	void ResetSpeeds();
	void ResetJumpVelocity();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBuffSpeed(float BaseSpeed, float CrouchSpeed);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastBuffJump(float JumpZVel);

private:	
	UPROPERTY()
	class AShooterCharacter* ShooterCharacter;
	
	/* Healing Attributes */
	bool bIsHealing = false;
	float HealthRate = 0.f;
	float HealAmt = 0.f;
	float HealthPerFrame = 0.f;

	/* Speed Attributes */
	float InitialBaseSpeed = 0.f;
	float InitialCrouchSpeed = 0.f;

	FTimerHandle SpeedTimerHandle;

	/* Jump Attributes */
	float InitialJumpZVel = 0.f;

	FTimerHandle JumpTimerHandle;
};
