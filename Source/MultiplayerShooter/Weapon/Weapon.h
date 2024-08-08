#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Weapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_InitialState UMETA(DisplayName = "InitialState"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),

	EWS_MAX UMETA(DisplayName = "MAX")
};

UCLASS()
class MULTIPLAYERSHOOTER_API AWeapon : public AActor
{
	GENERATED_BODY()

public:
	AWeapon();

	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty> &OutLifetimeProps) const override;

	void ShowPickupWidget(bool isEnabled);
	virtual void Fire(const FVector &HitLocation);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnEquipAreaOverlap(UPrimitiveComponent *OverlappedComponent,
									AActor *OtherActor,
									UPrimitiveComponent *OtherComp,
									int32 OtherBodyIndex,
									bool bFromSweep,
									const FHitResult &SweepResult);

	UFUNCTION()
	virtual void OnEquipAreaEndOverlap(
		UPrimitiveComponent *OverlappedComponent,
		AActor *OtherActor,
		UPrimitiveComponent *OtherComp,
		int32 OtherBodyIndex);

private:
	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USkeletalMeshComponent *WeaponMesh;

	UPROPERTY(VisibleAnywhere, Category = "Weapon Properties")
	class USphereComponent *EquipArea;

	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "Weapon Properties")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UWidgetComponent *PickupWidget;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	class UAnimationAsset *FireAnimation;

	UPROPERTY(EditAnywhere)
	TSubclassOf<class AAmmoShell> AmmoShellClass;

	UPROPERTY(EditAnywhere)
	float ZoomedFOV;

	UPROPERTY(EditAnywhere)
	float ZoomedInterpSpeed;

public:
	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	class UTexture2D *CrosshairCenter;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D *CrosshairTop;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D *CrosshairBottom;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D *CrosshairLeft;

	UPROPERTY(EditAnywhere, Category = "Crosshairs")
	UTexture2D *CrosshairRight;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	bool bIsAutomatic = false;

	UPROPERTY(EditAnywhere, Category = "Weapon")
	float FireDelay = 0.2f;

	void SetWeaponState(EWeaponState CurrentState);

	FORCEINLINE USkeletalMeshComponent *GetWeaponMesh() { return WeaponMesh; }
	FORCEINLINE float GetZoomedFOV() { return ZoomedFOV; }
	FORCEINLINE float GetZoomedInterpSpeed() { return ZoomedInterpSpeed; }
};
