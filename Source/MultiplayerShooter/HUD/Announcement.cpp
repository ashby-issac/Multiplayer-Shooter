// Fill out your copyright notice in the Description page of Project Settings.


#include "Announcement.h"
#include "Components/TextBlock.h"

void UAnnouncement::UpdateWarmupCountdownValue(float TotalSeconds)
{
    SetAnnouncementTimer(TotalSeconds);

    FString AnnounceString("Match Starts In:");
    AnnouncementText->SetText(FText::FromString(AnnounceString));

    FString InfoString("Fly Around: W, A, S, D");
    InfoText->SetText(FText::FromString(InfoString));
}

void UAnnouncement::UpdateCooldownAnnouncementHUD(float TotalSeconds)
{
    SetAnnouncementTimer(TotalSeconds);

    FString AnnounceString("New Match Starts In:");
    AnnouncementText->SetText(FText::FromString(AnnounceString));
}

void UAnnouncement::UpdateWinnersText(FString WinnerText)
{
    InfoText->SetText(FText::FromString(WinnerText));
}

void UAnnouncement::SetAnnouncementTimer(float TotalSeconds)
{
    if (TotalSeconds < 0.f)
    {
        WarmupTimer->SetText(FText());
        return;
    }

    int32 Minutes = FMath::FloorToInt32(TotalSeconds / 60.f);
    int32 Seconds = FMath::FloorToInt32(TotalSeconds) % 60;

    FString ValueString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    WarmupTimer->SetText(FText::FromString(ValueString));
}