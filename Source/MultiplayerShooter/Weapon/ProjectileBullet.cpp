// Fill out your copyright notice in the Description page of Project Settings.

#include "ProjectileBullet.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/Character.h"
#include "GameFramework/ProjectileMovementComponent.h"

AProjectileBullet::AProjectileBullet()
{
    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
    ProjectileMovementComponent->SetIsReplicated(true);
}

void AProjectileBullet::OnProjectileHit(UPrimitiveComponent *HitComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse, const FHitResult &Hit)
{
    ACharacter *OwnerCharacter = Cast<ACharacter>(GetOwner());

    if (OwnerCharacter != nullptr)
    {
        // Apply damage to server controlled character
        // Replicate the damage down to the specific or
        // same character present in the clients.
        UE_LOG(LogTemp, Warning, TEXT(":: OwnerCharacter is not null"));

        if (OwnerCharacter->Controller != nullptr)
        {
            UE_LOG(LogTemp, Warning, TEXT(":: ApplyingDamage"));
            UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerCharacter->Controller, this, UDamageType::StaticClass());
        }
    }
    UE_LOG(LogTemp, Warning, TEXT(":: OwnerCharacter is null"));

    Super::OnProjectileHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
