
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiplayerShooter/BlasterTypes/TurningInPlace.h"
#include "MultiplayerShooter/Interfaces/CrosshairsInteractor.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AShooterCharacter : public ACharacter, public ICrosshairsInteractor
{
	GENERATED_BODY()

public:
	AShooterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

protected:
	virtual void BeginPlay() override;
	virtual void Jump() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void LookRight(float Value);
	void OnCrouchPressed();


	void EquipWeapon();
	void OnAimPressed();
	void OnAimReleased();

	void OnFirePressed();
	void OnFireReleased();

private:
	UPROPERTY(EditAnywhere, Category = "Character Health")
	float MaxHealth = 100.f;

	UPROPERTY(ReplicatedUsing = OnRep_HealthDamaged)
	float Health = 100.f;

	UPROPERTY(VisibleAnywhere)
	class UCameraComponent *CameraComponent;

	UPROPERTY(VisibleAnywhere)
	class USpringArmComponent *SpringArmComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent *OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon *OverlappingWeapon;

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent *CombatComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage *FireMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage *HitReactMontage;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon *LastWeapon);

	UFUNCTION()
	void OnRep_HealthDamaged();

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed(); // Server RPC

	ETurningInPlace TurnInPlaceState;

	float AO_Yaw;
	float AO_Pitch;

	float Interp_AO;
	float CamThreshold = 200.f;
	float TurnThreshold = 0.5f;
	float ProxyRotYaw;
	float TimeForProxyRotation;

	bool bRotateRootBone = false;

	FRotator InitialAimRot;
	FRotator DeltaAimRot;
	FRotator ProxyRotationLastFrame;
	FRotator CurrentProxyRotation;

	void CalculateAimOffsets(float DeltaTime);
	void CheckForTurningInPlace(float DeltaTime);
	void CheckCamIsOnCloseContact();
	void DisableMeshesOnCloseContact(bool bActive);
	void PlayHitReactMontage();
	void SimulatedProxiesTurn();
	void CalculatePitch();

public:
	FVector GetCrosshairHitTarget();
	class AShooterPlayerController* ShooterController;
	class AShooterHUD* ShooterHUD;

	void Damage();

	void SetOverlappingWeapon(AWeapon *Weapon);

	bool IsWeaponEquipped();
	bool IsAiming();
	void PlayFireMontage(bool bAiming);
	AWeapon *GetEquippedWeapon();

	UFUNCTION(NetMulticast, Unreliable)
	void MulticastHit();

	FORCEINLINE float GetAO_Yaw() { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlaceState() { return TurnInPlaceState; }
	FORCEINLINE UCameraComponent *GetFollowCam() { return CameraComponent; }
	FORCEINLINE bool GetRotateRootBoneState() { return bRotateRootBone; }

	virtual void OnRep_ReplicatedMovement() override;

};
