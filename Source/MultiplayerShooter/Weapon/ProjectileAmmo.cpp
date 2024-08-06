
#include "ProjectileAmmo.h"
#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "MultiplayerShooter/Character/ShooterCharacter.h"
#include "Sound/SoundCue.h"

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
    }

    if (HasAuthority())
    {
        CollisionBox->OnComponentHit.AddDynamic(this, &ThisClass::OnProjectileHit);
    }
}

void AProjectileAmmo::OnProjectileHit(UPrimitiveComponent *HitComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse, const FHitResult &Hit)
{
    if (auto ShooterChar = Cast<AShooterCharacter>(OtherActor))
    {
        // Apply damage to server controlled character
        // Replicate the damage down to the specific or
        // same character present in the clients.
        // if (ShooterChar != nullptr)
        // {
        //     ShooterChar->Damage();
        // }

        ShooterChar->MulticastHit();
    }

    Destroy();
}

void AProjectileAmmo::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AProjectileAmmo::Destroyed()
{
    Super::Destroyed();

    if (ImpactParticles)
    {
        UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
    }

    if (ImpactSound)
    {
        UGameplayStatics::SpawnSoundAtLocation(this, ImpactSound, GetActorLocation());
    }
}