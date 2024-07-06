
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiplayerShooter/BlasterTypes/TurningInPlace.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AShooterCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AShooterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void LookRight(float Value);
	void OnCrouchPressed();

	virtual void Jump() override;

	void EquipWeapon();
	void OnAimPressed();
	void OnAimReleased();

	void OnFirePressed();
	void OnFireReleased();

private:
	UPROPERTY(VisibleAnywhere)
	class UCameraComponent* CameraComponent;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent* SpringArmComponent;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess="true"))
	class UWidgetComponent* OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon* LastWeapon);

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess="true"))
	class UAnimMontage* FireMontage;

	ETurningInPlace TurnInPlaceState;

	float AO_Yaw;
	float AO_Pitch;

	float Interp_AO;

	FRotator InitialAimRot;
	FRotator DeltaAimRot;

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed(); // Server RPC

	void CalculateAimOffsets(float DeltaTime);
	void CheckForTurningInPlace(float DeltaTime);

public:
	void SetOverlappingWeapon(AWeapon* Weapon);

	bool IsWeaponEquipped();
	bool IsAiming();
	void PlayFireMontage(bool bAiming);

	FORCEINLINE float GetAO_Yaw() { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlaceState() { return TurnInPlaceState; }

	AWeapon* GetEquippedWeapon();
};
