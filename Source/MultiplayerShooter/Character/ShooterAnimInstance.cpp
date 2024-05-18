
#include "ShooterAnimInstance.h"
#include "ShooterCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "MultiplayerShooter/Weapon/Weapon.h"

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

	AO_Yaw = ShooterCharacter->GetAO_Yaw();
	AO_Pitch = ShooterCharacter->GetAO_Pitch();

	EquippedWeapon = ShooterCharacter->GetEquippedWeapon();

	// Logic for Strafing
	CharacterStrafing(DeltaSeconds);
	
	// Logic for Leaning
	CharacterLeaning(DeltaSeconds);

	CalculateLeftHandTransform();
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
	YawOffset = -(CurrentDeltaRot.Yaw);

	 UE_LOG(LogTemp, Warning, TEXT(":: CharacterStrafing Velocity: %s"), *ShooterCharacter->GetVelocity().ToString());
	 UE_LOG(LogTemp, Warning, TEXT(":: CharacterStrafing Value: %s"), *CurrentDeltaRot.ToString());
}

void UShooterAnimInstance::CalculateLeftHandTransform()
{
	if (bWeaponEquipped && EquippedWeapon && EquippedWeapon->GetWeaponMesh() && ShooterCharacter->GetMesh())
	{
		// Getting the Weapon's socket in world space to position left hand 
		LeftHandTransform = EquippedWeapon->GetWeaponMesh()->GetSocketTransform(FName("LeftHandSocket"), 
																				ERelativeTransformSpace::RTS_World);
		FVector OutLocation;
		FRotator OutRotator;
		ShooterCharacter->GetMesh()->TransformToBoneSpace(FName("hand_r"),
														  LeftHandTransform.GetLocation(),
														  FRotator::ZeroRotator,
														  OutLocation,
														  OutRotator);
		LeftHandTransform.SetLocation(OutLocation);
		LeftHandTransform.SetRotation(FQuat(OutRotator));
	}
}
