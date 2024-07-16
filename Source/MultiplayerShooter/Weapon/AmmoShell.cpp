
#include "AmmoShell.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

AAmmoShell::AAmmoShell()
{
	PrimaryActorTick.bCanEverTick = false;

	ShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShellMesh"));
	RootComponent = ShellMesh;

	ShellMesh->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	ShellMesh->SetSimulatePhysics(true);
	ShellMesh->SetEnableGravity(true);
	ShellMesh->SetNotifyRigidBodyCollision(true);

	ShellImpulse = 10.f;
}

void AAmmoShell::BeginPlay()
{
	Super::BeginPlay();
	
	ShellMesh->AddImpulse(GetActorForwardVector() * ShellImpulse);

	ShellMesh->OnComponentHit.AddDynamic(this, &ThisClass::OnShellHit);
}

void AAmmoShell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AAmmoShell::OnShellHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (HitSFX)
	{
		UGameplayStatics::SpawnSoundAtLocation(this, HitSFX, GetActorLocation());
		HitSFX = nullptr;
	}

	FTimerHandle DestroyTimer;
	GetWorldTimerManager().SetTimer(DestroyTimer, this, &ThisClass::DestroyAmmo, 2.f);
}

void AAmmoShell::DestroyAmmo()
{
	Destroy();
}

