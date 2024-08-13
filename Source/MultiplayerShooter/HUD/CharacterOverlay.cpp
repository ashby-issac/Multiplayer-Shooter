// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterOverlay.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"

void UCharacterOverlay::UpdateHealthInfo(float Health, float MaxHealth)
{
    FString ValueString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health), 
                                                         FMath::CeilToInt(MaxHealth));
    HealthPercent->SetText(FText::FromString(ValueString));
    HealthBar->SetPercent(Health/MaxHealth);
}
