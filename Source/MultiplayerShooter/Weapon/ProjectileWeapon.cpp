// Fill out your copyright notice in the Description page of Project Settings.


#include "ProjectileWeapon.h"
#include "Components/BoxComponent.h"

AProjectileWeapon::AProjectileWeapon()
{
    PrimaryActorTick.bCanEverTick = false;

    // CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    // SetRootComponent(CollisionBox);
    // CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    // CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    // CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    // CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
    // CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);
}
	
void AProjectileWeapon::BeginPlay()
{
    Super::BeginPlay();
}

void AProjectileWeapon::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
