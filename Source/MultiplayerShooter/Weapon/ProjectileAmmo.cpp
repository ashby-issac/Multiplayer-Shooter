
#include "ProjectileAmmo.h"
#include "Sound/SoundCue.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/BoxComponent.h"
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
}

void AProjectileAmmo::BeginPlay()
{
    Super::BeginPlay();

    if (ProjectileTrace != nullptr)
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

void AProjectileAmmo::SpawnTrialSystem() 
{
    if (TrailSystem != nullptr)
    {
        TrailSystemComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(
            TrailSystem,
            GetRootComponent(),
            FName(),
            GetActorLocation(),
            GetActorRotation(),
            EAttachLocation::KeepWorldPosition,
            false);
    }
}

void AProjectileAmmo::SpawnTrialSFX()
{
    if (TrailSFX != nullptr && TrailAttenuation != nullptr)
    {
        TrailComponent = UGameplayStatics::SpawnSoundAttached(
            TrailSFX,
            GetRootComponent(),
            FName(),
            GetActorLocation(),
            GetActorRotation(),
            EAttachLocation::KeepWorldPosition,
            false,
            1.f,
            1.f,
            0.f,
            TrailAttenuation,
            (USoundConcurrency*)nullptr,
            false);
    }
}

void AProjectileAmmo::ApplyProjectileRadialDamage()
{
    APawn* FiringPawn = GetInstigator();
    if (FiringPawn != nullptr && HasAuthority())
    {
        AController* FiringController = FiringPawn->GetController();
        if (FiringController != nullptr)
        {
            UGameplayStatics::ApplyRadialDamageWithFalloff(
                this,
                Damage,
                MinDamage,
                GetActorLocation(),
                DamageInnerRadius,
                DamageOuterRadius,
                DamageFalloff,
                UDamageType::StaticClass(),
                TArray<AActor*>(),
                this,
                FiringController
            );
        }
    }
}

void AProjectileAmmo::StartDestroyTimer()
{
    GetWorldTimerManager().SetTimer(
        DestroyTimer,
        this,
        &ThisClass::OnDestroyProjectile,
        DestroyDelay);
}

void AProjectileAmmo::OnProjectileHit(UPrimitiveComponent *HitComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, FVector NormalImpulse, const FHitResult &Hit)
{
    Destroy();
}

void AProjectileAmmo::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void AProjectileAmmo::OnDestroyProjectile()
{
    Destroy();
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