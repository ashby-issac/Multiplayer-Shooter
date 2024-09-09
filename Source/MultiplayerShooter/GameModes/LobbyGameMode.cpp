// Fill out your copyright notice in the Description page of Project Settings.


#include "LobbyGameMode.h"
#include "GameFramework/GameState.h"

void ALobbyGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	if (GameState != nullptr)
	{
		int32 NumOfPlayers = GameState->PlayerArray.Num();
		if (NumOfPlayers >= 2)
		{
			UWorld* World = GetWorld();
			if (World)
			{
				bUseSeamlessTravel = true;
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1,
						15.f,
						FColor::Green,
						FString::Printf(TEXT("PostLoginSuccess"))
					);
				}
				World->ServerTravel(FString("/Game/Maps/Prototype?listen"));
			}
		}
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("PostLogin"))
		);
	}
}
