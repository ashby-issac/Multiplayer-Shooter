
#include "CharacterOverlay.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Math/UnrealMathUtility.h"

void UCharacterOverlay::UpdateHealthInfo(float Health, float MaxHealth)
{
    FString ValueString = FString::Printf(TEXT("%d/%d"), FMath::CeilToInt(Health),
                                          FMath::CeilToInt(MaxHealth));
    HealthPercent->SetText(FText::FromString(ValueString));
    HealthBar->SetPercent(Health / MaxHealth);
}

void UCharacterOverlay::UpdateScoreValue(float Score)
{
    FString ValueString = FString::Printf(TEXT("%d"), FMath::CeilToInt(Score));
    ScoreValue->SetText(FText::FromString(ValueString));
}

void UCharacterOverlay::UpdateDefeatValue(int32 Defeat)
{
    FString ValueString = FString::Printf(TEXT("%d"), Defeat);
    DefeatsValue->SetText(FText::FromString(ValueString));
}

void UCharacterOverlay::UpdateWeaponAmmoValue(int32 Ammo)
{
    FString ValueString = FString::Printf(TEXT("%d"), Ammo);
    WeaponAmmoValue->SetText(FText::FromString(ValueString));
}

void UCharacterOverlay::UpdateCarriedAmmoValue(int32 Ammo)
{
    FString ValueString = FString::Printf(TEXT("%d"), Ammo);
    CarriedAmmoValue->SetText(FText::FromString(ValueString));
}

void UCharacterOverlay::UpdateMatchCountdownValue(float TotalSeconds)
{
    if (TotalSeconds < 0.f)
    {
        MatchCountdownValue->SetText(FText());
        return;
    }

    int32 Minutes = FMath::FloorToInt32(TotalSeconds / 60.f);
    int32 Seconds = FMath::FloorToInt32(TotalSeconds) % 60;

    FString ValueString = FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds);
    MatchCountdownValue->SetText(FText::FromString(ValueString));
}

void UCharacterOverlay::UpdateGrenadesValue(int32 Grenades)
{
    FString ValueString = FString::Printf(TEXT("%d"), Grenades);
    GrenadesValue->SetText(FText::FromString(ValueString));
}