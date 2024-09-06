// Fill out your copyright notice in the Description page of Project Settings.


#include "Announcement.h"
#include "Components/TextBlock.h"

void UAnnouncement::UpdateWarmupCountdownValue(float TotalSeconds)
{
    int32 Minutes = FMath::FloorToInt32(TotalSeconds / 60.f);
    int32 Seconds = FMath::FloorToInt32(TotalSeconds) % 60;

    FString ValueString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    WarmupTimer->SetText(FText::FromString(ValueString));
}