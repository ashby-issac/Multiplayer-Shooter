#pragma once

UENUM(BlueprintType)
enum class EWeaponType : uint8
{
    EWT_AssaultRifle UMETA(DisplayName = "Assault Rifle"),
    EWT_RocketLaucher UMETA(DisplayName = "Rocket Laucher"),
    EWT_Pistol UMETA(DisplayName = "Pistol"),

    EWT_MAX UMETA(DisplayName = "DefaultMAX")
};
