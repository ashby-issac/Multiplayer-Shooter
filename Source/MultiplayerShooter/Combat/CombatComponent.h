#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CombatComponent.generated.h"

class AWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MULTIPLAYERSHOOTER_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	friend class AShooterCharacter;
	void EquipWeapon(AWeapon* WeaponToEquip);

	UFUNCTION(Server, Reliable)
	void ServerAimSync(bool bIsAiming);

protected:
	virtual void BeginPlay() override;

private:
	class AShooterCharacter* ShooterCharacter;

	UPROPERTY(ReplicatedUsing = OnRep_OnEquippedWeapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UFUNCTION()
	void OnRep_OnEquippedWeapon();

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

public:
	void SetAimingState(bool bIsAiming);
	FORCEINLINE AWeapon* GetEquippedWeapon() const { return EquippedWeapon; }

};
