
#include "BuffComponent.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"

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
	ShieldRampUp(DeltaTime);
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

void UBuffComponent::ShieldRampUp(float DeltaTime)
{
	if (!bShieldReplenishing || !ShooterCharacter || ShooterCharacter->GetIsEliminated()) return;

	ShieldReplenishPerFrame = ShieldReplenishRate * DeltaTime;
	ShooterCharacter->SetShield(FMath::Clamp(ShooterCharacter->GetShield() + ShieldReplenishPerFrame, 0.f, ShooterCharacter->GetMaxShield()));
	ShieldAmt -= ShieldReplenishPerFrame;
	ShooterCharacter->UpdatePlayerShieldHUD();

	if (ShieldAmt <= 0.f || ShooterCharacter->GetShield() >= ShooterCharacter->GetMaxShield())
	{
		 bShieldReplenishing = false;
		 ShieldAmt = 0.f;
	}
}

void UBuffComponent::Heal(float HealthAmt, float HealthTiming)
{
	bIsHealing = true;
	HealthRate = HealthAmt / HealthTiming;
	if (ShooterCharacter)
		HealAmt = FMath::Abs(HealthAmt - ShooterCharacter->GetHealth());
}

void UBuffComponent::ShieldReplenish(float ShieldReplenishAmt, float SheildReplenishTime)
{
	bShieldReplenishing = true;
	ShieldReplenishRate = ShieldReplenishAmt / SheildReplenishTime;
	if (ShooterCharacter)
		ShieldAmt = FMath::Abs(ShieldReplenishAmt - ShooterCharacter->GetShield());
}

void UBuffComponent::MulticastBuffSpeed_Implementation(float BaseSpeed, float CrouchSpeed)
{
	if (ShooterCharacter == nullptr || ShooterCharacter->GetCharacterMovement() == nullptr) return;

	ShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	ShooterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;
}

void UBuffComponent::BuffSpeed(float BaseSpeed, float CrouchSpeed, float SpeedDuration)
{
	if (ShooterCharacter == nullptr || ShooterCharacter->GetCharacterMovement() == nullptr) return;

	ShooterCharacter->GetWorldTimerManager().SetTimer(
		SpeedTimerHandle, 
		this, 
		&ThisClass::ResetSpeeds, 
		SpeedDuration);

	ShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = BaseSpeed;
	ShooterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = CrouchSpeed;

	// Call Multicast RPC in cases where client's version has different 
	// speed when compared to the character in the server
	MulticastBuffSpeed(BaseSpeed, CrouchSpeed);
}

void UBuffComponent::SetInitialSpeeds(float BaseSpeed, float CrouchSpeed)
{
	InitialBaseSpeed = BaseSpeed;
	InitialCrouchSpeed = CrouchSpeed;
}

void UBuffComponent::SetInitialJumpZVel(float JumpZVel)
{
	InitialJumpZVel = JumpZVel;
}

void UBuffComponent::ResetSpeeds()
{
	if (ShooterCharacter == nullptr || ShooterCharacter->GetCharacterMovement() == nullptr) return;

	ShooterCharacter->GetCharacterMovement()->MaxWalkSpeed = InitialBaseSpeed;
	ShooterCharacter->GetCharacterMovement()->MaxWalkSpeedCrouched = InitialCrouchSpeed;
	MulticastBuffSpeed(InitialBaseSpeed, InitialCrouchSpeed);
}

void UBuffComponent::BuffJump(float JumpZVelocity, float JumpVelocityDuration)
{
	if (ShooterCharacter == nullptr || ShooterCharacter->GetCharacterMovement() == nullptr) return;

	ShooterCharacter->GetWorldTimerManager().SetTimer(
		JumpTimerHandle,
		this,
		&ThisClass::ResetJumpVelocity,
		JumpVelocityDuration);

	ShooterCharacter->GetCharacterMovement()->JumpZVelocity = JumpZVelocity;
	MulticastBuffJump(JumpZVelocity);
}

void UBuffComponent::ResetJumpVelocity()
{
	if (ShooterCharacter == nullptr || ShooterCharacter->GetCharacterMovement() == nullptr) return;

	ShooterCharacter->GetCharacterMovement()->JumpZVelocity = InitialJumpZVel;
	MulticastBuffJump(InitialJumpZVel);
}

void UBuffComponent::MulticastBuffJump_Implementation(float JumpZVel)
{
	if (ShooterCharacter == nullptr || ShooterCharacter->GetCharacterMovement() == nullptr) return;

	ShooterCharacter->GetCharacterMovement()->JumpZVelocity = JumpZVel;
}
