#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MultiplayerShooter/HUD/ShooterHUD.h"
#include "MultiplayerShooter/Weapon/WeaponTypes.h"
#include "MultiplayerShooter/BlasterTypes/CombatTypes.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

class AWeapon;

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent))
class MULTIPLAYERSHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction *ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	friend class AShooterCharacter;
	void EquipWeapon(AWeapon *WeaponToEquip);

	UFUNCTION(Server, Reliable)
	void ServerAimSync(bool bIsAiming);

	void SetFiringState(bool isFiring);
	
	UFUNCTION(BlueprintCallable)
	void OnReloadFinished();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UFUNCTION()
	void OnRep_OnEquippedWeapon();

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION(Server, Reliable)
	void ServerFire(FVector_NetQuantize FireHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(FVector_NetQuantize FireHitTarget);

	void FindCrosshairHitTarget(FHitResult &HitResult);
	void HandleReload();

private:
	UPROPERTY()
	class AShooterCharacter *ShooterCharacter;

	UPROPERTY()
	class AShooterPlayerController *ShooterController;

	UPROPERTY()
	class AShooterHUD *ShooterHUD;

	UPROPERTY(ReplicatedUsing = OnRep_OnEquippedWeapon)
	AWeapon *EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere, ReplicatedUsing = OnRep_CarriedAmmo)
	int32 CarriedAmmo;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	UPROPERTY(EditAnywhere)
	float DefaultZoomFOV;

	UPROPERTY(EditAnywhere)
	float DefaultZoomInterpSpeed;

	UPROPERTY(EditAnywhere)
	int32 ARInitialAmmo;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	TMap<EWeaponType, int32> CarriedAmmoMap;

	bool bIsFireBtnPressed;
	bool bCanFire = true;
	float CrosshairInAirFactor, CrosshairAimFactor;
	float CurrentFOV;

	FHitResult CrosshairHitResult;
	FHUDPackage HUDPackage;
	FTimerHandle FireRateTimerHandle;

	void ReloadWeapon();
	void SetCrosshairsForWeapon(float DeltaTime);
	void SetZoomedFOV(float DeltaTime);
	void EnableAutomaticFiring();
	void Fire();
	void OnFireDelayed();
	bool CanFire();
	void InitializeCarriedAmmo();
	void UpdateCarriedAmmoHUD();

public:
	FVector CrosshairHitTarget;

	void SetAimingState(bool bIsAiming);
	FORCEINLINE AWeapon *GetEquippedWeapon() const { return EquippedWeapon; }
	FORCEINLINE int32 GetCarriedAmmo() const { return CarriedAmmo; }
};
