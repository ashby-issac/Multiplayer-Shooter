
#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UShooterAnimInstance::UShooterAnimInstance()
{

}

void UShooterAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
}

void UShooterAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (!ShooterCharacter)
	{
		ShooterCharacter = Cast<AShooterCharacter>(TryGetPawnOwner());
	}

	if (!ShooterCharacter) { return; }
	
	UE_LOG(LogTemp, Warning, TEXT(":: Inside NativeUpdateAnimation"));

	Velocity = ShooterCharacter->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = ShooterCharacter->GetCharacterMovement()->IsFalling();
	bIsAccelerating = ShooterCharacter->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0.f ? true : false;
	bWeaponEquipped = ShooterCharacter->IsWeaponEquipped();
	bIsCrouched = ShooterCharacter->bIsCrouched;
	bIsAiming = ShooterCharacter->IsAiming();

	// Logic for Strafing
	CharacterStrafing(DeltaSeconds);
	
	// Logic for Leaning
	CharacterLeaning(DeltaSeconds);
}

void UShooterAnimInstance::CharacterLeaning(float DeltaSeconds)
{
	RotationLastFrame = CurrentRotation;
	CurrentRotation = ShooterCharacter->GetActorRotation();
	const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(CurrentRotation, RotationLastFrame);
	const float Target = DeltaRotation.Yaw / DeltaSeconds;
	const float InterpValue = FMath::FInterpTo(LeanOffset, Target, DeltaSeconds, 6.f);
	LeanOffset = FMath::Clamp(InterpValue, -90.f, 90.f);
}

void UShooterAnimInstance::CharacterStrafing(float DeltaSeconds)
{
	FRotator AimRotation = ShooterCharacter->GetBaseAimRotation();
	FRotator MoveRotation = UKismetMathLibrary::MakeRotFromX(ShooterCharacter->GetVelocity());
	FRotator NewDeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(AimRotation, MoveRotation);
	CurrentDeltaRot = UKismetMathLibrary::RInterpTo(CurrentDeltaRot, NewDeltaRot, DeltaSeconds, 6.f);
	YawOffset = CurrentDeltaRot.Yaw;
}
