
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "MultiplayerShooter/BlasterTypes/TurningInPlace.h"
#include "MultiplayerShooter/Interfaces/CrosshairsInteractor.h"
#include "Components/TimelineComponent.h"
#include "MultiplayerShooter/BlasterTypes/CombatTypes.h"
#include "ShooterCharacter.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AShooterCharacter : public ACharacter, public ICrosshairsInteractor
{
	GENERATED_BODY()

public:
	AShooterCharacter();

	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent *PlayerInputComponent) override;
	virtual void Destroyed() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;
	virtual void PostInitializeComponents() override;

	UPROPERTY()
	class AShooterPlayerController *ShooterPlayerController;
	
	UPROPERTY()
	class AShooterHUD *ShooterHUD;

	bool IsWeaponEquipped();
	bool IsAiming();
	void SetOverlappingWeapon(AWeapon *Weapon);
	void PlayFireMontage(bool bAiming);
	void PlayReloadMontage();
	void PlayGrenadeThrowMontage();
	void UpdatePlayerHealthHUD();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastEliminate();
	void OnEliminated();

	AWeapon *GetEquippedWeapon();
	FVector GetCrosshairHitTarget();

	virtual void OnRep_ReplicatedMovement() override;
	ECombatState GetCombatState() const; 

	UFUNCTION(BlueprintImplementableEvent)
	void ShowSniperScope(bool bVisibility);

protected:
	virtual void BeginPlay() override;
	virtual void Jump() override;

	void MoveForward(float Value);
	void MoveRight(float Value);
	void LookUp(float Value);
	void LookRight(float Value);
	void OnCrouchPressed();
	void OnReloadPressed();

	void EquipWeapon();
	void OnAimPressed();
	void OnAimReleased();

	void OnFirePressed();
	void OnFireReleased();

	void OnGrenadeThrowPressed();

	void RespawnCharacter();

	UFUNCTION()
	void OnRep_OverlappingWeapon(AWeapon *LastWeapon);

	UFUNCTION()
	void OnRep_HealthDamaged(float PrevHealth);

	UFUNCTION(Server, Reliable)
	void ServerEquipButtonPressed(); // Server RPC

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

	UPROPERTY(VisibleAnywhere)
	class UStaticMeshComponent* GrenadeMesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent *OverheadWidget;

	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AWeapon *OverlappingWeapon;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UCombatComponent *CombatComponent;

	UPROPERTY(VisibleAnywhere)
	class UBuffComponent* BuffComponent;

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

	UPROPERTY(EditAnywhere)
	class UParticleSystem *ElimBotParticle;

	UPROPERTY(VisibleAnywhere)
	class UParticleSystemComponent *ElimBotComponent;

	UPROPERTY(EditAnywhere)
	class USoundCue *ElimBotSoundCue;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UAnimMontage *FireWeaponMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAnimMontage *HitReactMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAnimMontage *ElimMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAnimMontage* GrenadeThrowMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	UAnimMontage *ReloadMontage;

	UPROPERTY()
	class AShooterPlayerState *ShooterPlayerState;

	FOnTimelineFloat DissolveTrack; // Dynamic Delegate
	ETurningInPlace TurnInPlaceState;
	FTimerHandle ElimTimerHandle;

	float AO_Yaw;
	float AO_Pitch;
	float ElimBotZOffset = 200.f;
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
	void PollInit();
	void RotateInPlace(float DeltaTime);

public:
	UPROPERTY(Replicated)
	bool bDisableGameplay = false;

	// 	FORCEINLINE functions should be kept after their respective private variables
	FORCEINLINE float GetAO_Yaw() { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() { return AO_Pitch; }
	FORCEINLINE float GetHealth() { return Health; }
	FORCEINLINE void SetHealth(float NewHealth) { Health = NewHealth; }
	FORCEINLINE float GetMaxHealth() { return MaxHealth; }
	
	FORCEINLINE bool GetRotateRootBoneState() { return bRotateRootBone; }
	FORCEINLINE bool GetIsEliminated() { return bIsEliminated; }

	FORCEINLINE UAnimMontage* GetReloadMontage() { return ReloadMontage; }
	FORCEINLINE ETurningInPlace GetTurningInPlaceState() { return TurnInPlaceState; }
	FORCEINLINE UCameraComponent *GetFollowCam() { return CameraComponent; }
	FORCEINLINE UCombatComponent* GetCombatComponent() { return CombatComponent; }
	FORCEINLINE UBuffComponent* GetBuffComponent() const { return BuffComponent; }
	FORCEINLINE UStaticMeshComponent* GetGrenadeMesh() { return GrenadeMesh; }
};
