// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "OverheadWidget.generated.h"

/**
 * 
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UOverheadWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void OnLevelRemovedFromWorld(ULevel* Level, UWorld* World) override; // Double Check

private:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* OverheadText;

	UFUNCTION(BlueprintCallable)
	void SetNetworkRole(APawn* PlayerPawn);
	void SetOverheadText(FString DisplayText);
};
