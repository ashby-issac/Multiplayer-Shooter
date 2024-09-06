// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "ShooterHUD.generated.h"

USTRUCT(BlueprintType)
struct FHUDPackage
{
	GENERATED_BODY()

public:
	class UTexture2D *CrosshairCenter;
	UTexture2D *CrosshairTop;
	UTexture2D *CrosshairBottom;
	UTexture2D *CrosshairLeft;
	UTexture2D *CrosshairRight;
	float SpreadFactor;
	FLinearColor CrosshairColor;
};

class UCharacterOverlay;

UCLASS()
class MULTIPLAYERSHOOTER_API AShooterHUD : public AHUD
{
	GENERATED_BODY()

public:
	UPROPERTY()
	UCharacterOverlay *CharacterOverlay;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UCharacterOverlay> CharacterOverlayClass;

	UPROPERTY()
	class UAnnouncement *AnnouncementOverlay;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAnnouncement> AnnouncementOverlayClass;

	void AddCharacterOverlay();
	void AddAnnouncementOverlay();
	virtual void DrawHUD() override;

protected:
	virtual void BeginPlay() override;

private:
	FHUDPackage HUDPackage;

	UPROPERTY(EditAnywhere)
	float CrosshairMultiplier = 10.f;

	void DrawCrosshairToScreen(UTexture2D *CrosshairTexture, FVector2D ViewportCenter, FVector2D SpreadFactor, FLinearColor CrosshairColor);

public:
	FORCEINLINE void SetHUDPackageProps(FHUDPackage Package) { HUDPackage = Package; };
};
