// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "AmmoShell.generated.h"

UCLASS()
class MULTIPLAYERSHOOTER_API AAmmoShell : public AActor
{
	GENERATED_BODY()

public:
	AAmmoShell();
	virtual void Tick(float DeltaTime) override;

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	void OnShellHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

private:
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent *ShellMesh;

	UPROPERTY(EditAnywhere)
	class USoundCue* HitSFX;

	UPROPERTY(EditDefaultsOnly)
	float ShellImpulse;

	void DestroyAmmo();
};
