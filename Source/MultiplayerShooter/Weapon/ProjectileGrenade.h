
#pragma once

#include "CoreMinimal.h"
#include "ProjectileAmmo.h"
#include "ProjectileGrenade.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AProjectileGrenade : public AProjectileAmmo
{
	GENERATED_BODY()

public:
	AProjectileGrenade();
	virtual void Destroyed() override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnBounce(const FHitResult& ImpactResult, const FVector& ImpactVelocity);

private:
	UPROPERTY(EditAnywhere)
	class USoundCue* BounceSFX;
};
