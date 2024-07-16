
#include "AmmoShell.h"

AAmmoShell::AAmmoShell()
{
	PrimaryActorTick.bCanEverTick = false;

	ShellMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ShellMesh"));
	RootComponent = ShellMesh;
}

void AAmmoShell::BeginPlay()
{
	Super::BeginPlay();
	
}

void AAmmoShell::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

