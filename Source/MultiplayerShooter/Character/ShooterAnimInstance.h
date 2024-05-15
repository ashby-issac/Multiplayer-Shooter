// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ShooterAnimInstance.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API UShooterAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UShooterAnimInstance();
	
	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

	void CharacterLeaning(float DeltaSeconds);

	void CharacterStrafing(float DeltaSeconds);

private:
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	class AShooterCharacter* ShooterCharacter;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float Speed;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsInAir;

	UPROPERTY(BlueprintReadOnly, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	bool bIsAccelerating;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bWeaponEquipped;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bIsCrouched;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	bool bIsAiming;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float YawOffset;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float LeanOffset;
	
	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float AO_Yaw;

	UPROPERTY(BlueprintReadOnly, Category = "Character", meta = (AllowPrivateAccess = "true"))
	float AO_Pitch;

	FRotator CurrentDeltaRot;
	FRotator RotationLastFrame;
	FRotator CurrentRotation;

	FVector Velocity;
};
