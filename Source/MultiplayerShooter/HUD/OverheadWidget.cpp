
#include "OverheadWidget.h"
#include "Components/TextBlock.h"

void UOverheadWidget::OnLevelRemovedFromWorld(ULevel* Level, UWorld* World)
{
	RemoveFromParent();
	Super::OnLevelRemovedFromWorld(Level, World);
}

void UOverheadWidget::SetNetworkRole(APawn* PlayerPawn)
{
	ENetRole PawnRole = PlayerPawn->GetLocalRole();
	FString PawnRoleText;
	switch (PawnRole)
	{
		case ENetRole::ROLE_Authority:
			PawnRoleText = FString("Authority");
			break;
		case ENetRole::ROLE_SimulatedProxy:
			PawnRoleText = FString("SimulatedProxy");
			break;
		case ENetRole::ROLE_AutonomousProxy:
			PawnRoleText = FString("AutonomousProxy");
			break;
		case ENetRole::ROLE_None:
			PawnRoleText = FString("None");
			break;
	}

	FString RoleString = FString::Printf(TEXT("LocalRole: %s"), *PawnRoleText);
	SetOverheadText(RoleString);
}

void UOverheadWidget::SetOverheadText(FString DisplayText)
{
	if (!OverheadText) { return; }

	OverheadText->SetText(FText::FromString(DisplayText));
}
