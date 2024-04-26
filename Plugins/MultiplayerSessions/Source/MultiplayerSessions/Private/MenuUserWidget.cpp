// Fill out your copyright notice in the Description page of Project Settings.


#include "MenuUserWidget.h"
#include "Components/Button.h"
#include "OnlineSessionSettings.h"
#include "MultiplayerSessionsSubsystem.h"

void UMenuUserWidget::MenuSetup(int32 NumOfConnections, FString MatchType1, FString LobbyPath)
{
	AddToViewport();
	SetVisibility(ESlateVisibility::Visible);
	bIsFocusable = true;
	PathToLobby = FString::Printf(TEXT("%s?listen"), *LobbyPath);

	if (auto World = GetWorld())
	{
		if (auto PlayerController = World->GetFirstPlayerController())
		{
			FInputModeUIOnly UIInputMode;
			UIInputMode.SetWidgetToFocus(TakeWidget());
			UIInputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
			PlayerController->SetInputMode(UIInputMode);
		}
	}

	UGameInstance* GameInstance = GetGameInstance();
	if (GameInstance)
	{
		NumOfPublicConnections = NumOfConnections;
		MatchType = MatchType1;
		MultiplayerSessionsSubsystem = GameInstance->GetSubsystem<UMultiplayerSessionsSubsystem>();
	}

	if (MultiplayerSessionsSubsystem)
	{
		MultiplayerSessionsSubsystem->MPCreateSessionCompleteDelegate.AddDynamic(this, &ThisClass::OnCreateSessionComplete);
		MultiplayerSessionsSubsystem->MPFindSessionsCompleteDelegate.AddUObject(this, &ThisClass::OnFindSessionsComplete);
		MultiplayerSessionsSubsystem->MPJoinSessionCompleteDelegate.AddUObject(this, &ThisClass::OnJoinSessionComplete);
		MultiplayerSessionsSubsystem->MPStartSessionCompleteDelegate.AddDynamic(this, &ThisClass::OnStartSessionComplete);
		MultiplayerSessionsSubsystem->MPDestroySessionCompleteDelegate.AddDynamic(this, &ThisClass::OnDestroySessionComplete);
	}
}

void UMenuUserWidget::OnHostButtonClicked()
{
	if (MultiplayerSessionsSubsystem)
	{
		HostButton->SetIsEnabled(false);
		MultiplayerSessionsSubsystem->CreateGameSession(NumOfPublicConnections, MatchType);
	}
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Black,
			FString::Printf(TEXT("Host Button Clicked"))
		);
	}
}

void UMenuUserWidget::OnJoinButtonClicked()
{
	if (MultiplayerSessionsSubsystem)
	{
		JoinButton->SetIsEnabled(false);
		MultiplayerSessionsSubsystem->FindGameSession(10000);
	}

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Black,
			FString::Printf(TEXT("Join Button Clicked"))
		);
	}
}

bool UMenuUserWidget::Initialize()
{
	if (!Super::Initialize())
	{
		return false;
	}

	if (HostButton)
	{
		HostButton->OnClicked.AddDynamic(this, &ThisClass::OnHostButtonClicked);
	}

	if (JoinButton)
	{
		JoinButton->OnClicked.AddDynamic(this, &ThisClass::OnJoinButtonClicked);
	}

	return true;
}

void UMenuUserWidget::OnLevelRemovedFromWorld(ULevel* Level, UWorld* World)
{
	MenuTearDown();

	Super::OnLevelRemovedFromWorld(Level, World);
}

void UMenuUserWidget::OnCreateSessionComplete(bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		if (auto World = GetWorld())
		{
			World->ServerTravel(PathToLobby);
		}
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Green,
				FString::Printf(TEXT("Successfully Created Session"))
			);
		}
	}
	else
	{
		HostButton->SetIsEnabled(true);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Failed to create session"))
			);
		}
	}
}

void UMenuUserWidget::OnFindSessionsComplete(const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful)
{
	if (bWasSuccessful)
	{
		for (FOnlineSessionSearchResult SearchResult : SearchResults)
		{
			if (!SearchResult.IsValid())
			{
				continue;
			}

			FString TypeOfMatch;
			SearchResult.Session.SessionSettings.Get("MatchType", TypeOfMatch);
			if (MatchType.Equals(TypeOfMatch))
			{
				if (GEngine)
				{
					GEngine->AddOnScreenDebugMessage(
						-1,
						15.f,
						FColor::Green,
						FString::Printf(TEXT("Found a game session, calling join game session"))
					);
				}
				MultiplayerSessionsSubsystem->JoinGameSession(SearchResult);
				break;
			}
		}
	}
	else
	{
		JoinButton->SetIsEnabled(true);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Couldn't find game session, so not able to join"))
			);
		}
	}
}

void UMenuUserWidget::OnJoinSessionComplete(EOnJoinSessionCompleteResult::Type Result, FString ConnectString)
{
	if (Result == EOnJoinSessionCompleteResult::Success)
	{
		if (auto World = GetWorld())
		{
			if (auto PlayerController = World->GetFirstPlayerController())
			{
				PlayerController->ClientTravel(ConnectString, ETravelType::TRAVEL_Absolute);
			}
		}
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Green,
				FString::Printf(TEXT("Successfully joined session"))
			);
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Green,
				FString::Printf(TEXT("Failed to join session"))
			);
		}
	}
}

void UMenuUserWidget::OnStartSessionComplete(bool bWasSuccessful)
{
	// bind functionality
}

void UMenuUserWidget::OnDestroySessionComplete(bool bWasSuccessful)
{
	// bind functionality
}

void UMenuUserWidget::MenuTearDown()
{
	RemoveFromParent();

	if (auto World = GetWorld())
	{
		if (auto PlayerController = World->GetFirstPlayerController())
		{
			FInputModeGameOnly GameInputMode;
			PlayerController->SetInputMode(GameInputMode);
		}
	}
}
