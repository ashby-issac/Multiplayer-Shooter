#pragma once

UENUM(BlueprintType)
enum class ECombatState : uint8
{
	ECS_Unoccupied UMETA(DisplayName = "Unoccupied"),
	ECS_Reloading UMETA(DisplayName = "Reloading"),
	ECS_GrenadeThrow UMETA(DisplayName = "Grenade Throw"),

	ETIP_MAX UMETA(DisplayName = "DEFAULT MAX")
};