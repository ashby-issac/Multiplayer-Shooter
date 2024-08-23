
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiplayerShooter/BlasterTypes/TurningInPlace.h"
#include "MultiplayerShooter/Interfaces/CrosshairsInteractor.h"
#include "Components/TimelineComponent.h"
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

	class AShooterPlayerController *ShooterController;
	class AShooterHUD *ShooterHUD;

	bool IsWeaponEquipped();
	bool IsAiming();
	void SetOverlappingWeapon(AWeapon *Weapon);
	void PlayFireMontage(bool bAiming);

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate();
	void OnEliminated();

	AWeapon *GetEquippedWeapon();
	FVector GetCrosshairHitTarget();

	virtual void OnRep_ReplicatedMovement() override;

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

	void RespawnCharacter();

	UFUNCTION()
	void ReceiveDamage(AActor *DamagedActor, float Damage, const UDamageType *DamageType, AController *InstigatedBy, AActor *DamageCauser);

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage *ElimMontage;

	UPROPERTY(EditDefaultsOnly)
	float ElimDelay = 3.f;

	UPROPERTY(VisibleAnywhere, Category = "Elim")
	class UTimelineComponent *DissolveTimeline;

	UPROPERTY(EditAnywhere, Category = "Elim")
	UCurveFloat *DissolveCurve;

	UPROPERTY(EditAnywhere, Category = "Elim")
	UMaterialInstance *DissolveMaterialInstance;

	UPROPERTY(VisibleAnywhere, Category = "Elim")
	UMaterialInstanceDynamic *DynamicDissolveMaterialInstance;

	FOnTimelineFloat DissolveTrack; // Dynamic Delegate

	ETurningInPlace TurnInPlaceState;
	FTimerHandle ElimTimerHandle;

	float AO_Yaw;
	float AO_Pitch;

	float Interp_AO;
	float CamThreshold = 200.f;
	float TurnThreshold = 0.5f;
	float ProxyRotYaw;
	float TimeForProxyRotation;

	bool bRotateRootBone = false;
	bool bIsEliminated = false;

	FRotator InitialAimRot;
	FRotator DeltaAimRot;
	FRotator ProxyRotationLastFrame;
	FRotator CurrentProxyRotation;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon *LastWeapon);

	UFUNCTION()
	void OnRep_HealthDamaged();

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed(); // Server RPC

	UFUNCTION()
	void UpdateDissolveMaterial(float DissolveValue);

	void StartDissolve();
	void CalculateAimOffsets(float DeltaTime);
	void CheckForTurningInPlace(float DeltaTime);
	void CheckCamIsOnCloseContact();
	void DisableMeshesOnCloseContact(bool bActive);
	void PlayHitReactMontage();
	void PlayElimMontage();
	void SimulatedProxiesTurn();
	void CalculatePitch();
	void UpdatePlayerHUD();

public:
	// 	FORCEINLINE functions should be kept after the respective private variables
	FORCEINLINE float GetAO_Yaw() { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() { return AO_Pitch; }
	FORCEINLINE ETurningInPlace GetTurningInPlaceState() { return TurnInPlaceState; }
	FORCEINLINE UCameraComponent *GetFollowCam() { return CameraComponent; }
	FORCEINLINE bool GetRotateRootBoneState() { return bRotateRootBone; }
	FORCEINLINE bool GetIsEliminated() { return bIsEliminated; }
};
