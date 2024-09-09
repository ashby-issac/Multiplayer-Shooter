// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Announcement.generated.h"

/**
 *
 */
UCLASS()
class MULTIPLAYERSHOOTER_API UAnnouncement : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock *WarmupTimer;

	UPROPERTY(meta = (BindWidget))
	UTextBlock *AnnouncementText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock *InfoText;

	void UpdateWarmupCountdownValue(float TotalSeconds);
	void UpdateCooldownAnnouncementHUD(float TotalSeconds);
	void UpdateWinnersText(FString WinnerText);

private:
	void SetAnnouncementTimer(float TotalSeconds);
};
