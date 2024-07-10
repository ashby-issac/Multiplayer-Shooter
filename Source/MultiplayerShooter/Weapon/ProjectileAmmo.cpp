
#include "ProjectileAmmo.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"

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
    CollisionBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);

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

        // CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);
    }
}

void AProjectileAmmo::OnProjectileHit(UPrimitiveComponent *HitComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse, const FHitResult &Hit)
{
    FName Auth = HasAuthority() ? FName("HasAuthority") : FName("HasNoAuthority");

    UE_LOG(LogTemp, Warning, TEXT(":: OnProjectileHit"));
    if (auto ShooterChar = Cast<AShooterCharacter>(OtherActor))
    {
        // if HasAuthority()
        // Apply damage to server controlled character
        // Replicate the damage down to the specific or
        // same character present in the clients.
        if (ShooterChar != nullptr && HasAuthority())
        {
            ShooterChar->Damage();
        }
    }
}

void AProjectileAmmo::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}
