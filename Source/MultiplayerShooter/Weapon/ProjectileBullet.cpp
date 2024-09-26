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

        if (OwnerCharacter->Controller != nullptr)
        {
            UGameplayStatics::ApplyDamage(OtherActor, Damage, OwnerCharacter->Controller, this, UDamageType::StaticClass());
        }
    }

    Super::OnProjectileHit(HitComponent, OtherActor, OtherComp, NormalImpulse, Hit);
}
