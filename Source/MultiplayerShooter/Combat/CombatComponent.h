#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MultiplayerShooter/HUD/ShooterHUD.h"
#include "MultiplayerShooter/Weapon/WeaponTypes.h"
#include "MultiplayerShooter/BlasterTypes/CombatTypes.h"
#include "CombatComponent.generated.h"

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

	void SetFiringState(bool isFiring);
	void SetAimingState(bool bIsAiming);
	
	UFUNCTION(BlueprintCallable)
	void OnReloadFinished();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnRep_OnEquippedWeapon();

	UFUNCTION()
	void OnRep_CombatState();

	UFUNCTION()
	void OnRep_CarriedAmmo();

	UFUNCTION(Server, Reliable)
	void ServerFire(FVector_NetQuantize FireHitTarget);

	UFUNCTION(Server, Reliable)
	void ServerAimSync(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void ServerReload();

	UFUNCTION(NetMulticast, Reliable)
	void MulticastFire(FVector_NetQuantize FireHitTarget);

	void FindCrosshairHitTarget(FHitResult &HitResult);
	void HandleReload();
	void CalculateReloading();

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

	UPROPERTY(EditAnywhere, Category = "Starting Ammos")
	int32 InitialRifleAmmo;

	UPROPERTY(EditAnywhere, Category = "Starting Ammos")
	int32 InitialRocketAmmo;

	UPROPERTY(EditAnywhere, Category = "Starting Ammos")
	int32 InitialPistolAmmo;

	UPROPERTY(EditAnywhere, Category = "Starting Ammos")
	int32 InitialSMGAmmo;

	UPROPERTY(EditAnywhere, Category = "Starting Ammos")
	int32 InitialShotgunAmmo;

	UPROPERTY(EditAnywhere, Category = "Starting Ammos")
	int32 InitialSniperAmmo;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_CombatState)
	ECombatState CombatState = ECombatState::ECS_Unoccupied;

	bool bIsFireBtnPressed;
	bool bCanFire = true;
	float CrosshairInAirFactor, CrosshairAimFactor;
	float CurrentFOV;

	FHitResult CrosshairHitResult;
	FHUDPackage HUDPackage;
	FTimerHandle FireRateTimerHandle;
	TMap<EWeaponType, int32> CarriedAmmoMap;

	void ReloadWeapon();
	void SetCrosshairsForWeapon(float DeltaTime);
	void SetZoomedFOV(float DeltaTime);
	void EnableFiringEvent();
	void Fire();
	void OnFireDelayed();
	bool CanFire();
	void InitializeCarriedAmmo();
	void UpdateCarriedAmmoHUD();

public:
	FVector CrosshairHitTarget;

	FORCEINLINE AWeapon *GetEquippedWeapon() const { return EquippedWeapon; }
	FORCEINLINE int32 GetCarriedAmmo() const { return CarriedAmmo; }
};
