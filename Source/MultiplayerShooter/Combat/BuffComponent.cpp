
#include "BuffComponent.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"

UBuffComponent::UBuffComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

}


void UBuffComponent::BeginPlay()
{
	Super::BeginPlay();

}


void UBuffComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	HealRampUp(DeltaTime);
}

void UBuffComponent::HealRampUp(float DeltaTime)
{
	if (!bIsHealing || !ShooterCharacter || ShooterCharacter->GetIsEliminated()) return;

	HealthPerFrame = HealthRate * DeltaTime;
	ShooterCharacter->SetHealth(FMath::Clamp(ShooterCharacter->GetHealth() + HealthPerFrame, 0.f, ShooterCharacter->GetMaxHealth()));
	HealAmt -= HealthPerFrame;
	ShooterCharacter->UpdatePlayerHealthHUD();

	if (HealAmt <= 0.f || ShooterCharacter->GetHealth() >= ShooterCharacter->GetMaxHealth())
	{
		bIsHealing = false;
		HealAmt = 0;
	}
}

void UBuffComponent::Heal(float HealthAmt, float HealthTiming)
{
	bIsHealing = true;
	HealthRate = HealthAmt / HealthTiming;
	if (ShooterCharacter)
		HealAmt = FMath::Abs(HealthAmt - ShooterCharacter->GetHealth());
}