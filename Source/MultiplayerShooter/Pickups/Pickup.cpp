
#include "Pickup.h"
#include "Sound/SoundCue.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "MultiplayerShooter/Weapon/WeaponTypes.h"

APickup::APickup()
{
	PrimaryActorTick.bCanEverTick = true;

	bReplicates = true;

	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	
	OverlapSphere = CreateDefaultSubobject<USphereComponent>(TEXT("Overlap Sphere"));
	OverlapSphere->SetupAttachment(RootComponent);

	OverlapSphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	OverlapSphere->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	OverlapSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);
	OverlapSphere->SetSphereRadius(100.f);

	PickupMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Pickup Mesh"));
	PickupMesh->SetupAttachment(OverlapSphere);

	HealthPickupComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("HealthPickupComponent"));
	HealthPickupComponent->SetupAttachment(RootComponent);

	PickupMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PickupMesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);

	PickupMesh->SetRenderCustomDepth(true);
	PickupMesh->CustomDepthStencilValue = CUSTOM_DEPTH_BLUE;

	RootComponent->AddLocalOffset(FVector(0.f, 0.f, 100.f));
	PickupMesh->SetWorldRotation(FRotator(-90.f, 0.f, 0.F));
	PickupMesh->SetWorldScale3D(PickupScale);
}

void APickup::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority())
	{
		OverlapSphere->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnPickupAreaOverlap);
	}
}

void APickup::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void APickup::Destroyed()
{
	Super::Destroyed();

	if (PickupSFX != nullptr)
	{
		UGameplayStatics::PlaySoundAtLocation(
			this,
			PickupSFX,
			GetActorLocation());
	}

	if (HealthPickupSystem != nullptr)
	{
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(
			this,
			HealthPickupSystem,
			GetActorLocation(),
			GetActorRotation()
		);
	}
}

void APickup::OnPickupAreaOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Black,
			FString::Printf(TEXT("Pickup"))
		);
	}
}

