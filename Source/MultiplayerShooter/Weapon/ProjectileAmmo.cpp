
#include "ProjectileAmmo.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectileAmmo::AProjectileAmmo()
{
    PrimaryActorTick.bCanEverTick = false;

    bReplicates = true;
    CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
    SetRootComponent(CollisionBox);

    CollisionBox->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
    CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);

    CollisionBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
    CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
    CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldStatic, ECollisionResponse::ECR_Block);

    ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
    ProjectileMovementComponent->bRotationFollowsVelocity = true;
}

void AProjectileAmmo::BeginPlay()
{
    Super::BeginPlay();

    if (ProjectileTrace)
    {
        ProjectileTraceComponent = UGameplayStatics::SpawnEmitterAttached(
            ProjectileTrace,
            CollisionBox,
            FName(),
            GetActorLocation(),
            GetActorRotation(),
            EAttachLocation::KeepWorldPosition);
    }
}

void AProjectileAmmo::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
