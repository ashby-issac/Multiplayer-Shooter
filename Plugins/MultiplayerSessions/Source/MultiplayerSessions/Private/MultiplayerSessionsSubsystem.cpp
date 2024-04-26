// Fill out your copyright notice in the Description page of Project Settings.


#include "MultiplayerSessionsSubsystem.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"

UMultiplayerSessionsSubsystem::UMultiplayerSessionsSubsystem() :
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionCompleteDelegate)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsCompleteDelegate)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnJoinSessionCompleteDelegate)),
	StartSessionCompleteDelegate(FOnStartSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnStartSessionCompleteDelegate)),
	DestroySessionCompleteDelegate(FOnDestroySessionCompleteDelegate::CreateUObject(this, &ThisClass::OnDestroySessionCompleteDelegate))
{
	if (auto OnlineSubsystem = IOnlineSubsystem::Get())
	{
		SessionInterface = OnlineSubsystem->GetSessionInterface();
		if (SessionInterface && GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Connected to steam: %s"), *OnlineSubsystem->GetSubsystemName().ToString())
			);
		}
	}
}

void UMultiplayerSessionsSubsystem::CreateGameSession(int32 NumOfConnections, FString MatchType)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	auto ExistingSession = SessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		IsSessionDestroyed = false;
		LastNumOfConnections = NumOfConnections;
		LastMatchType = MatchType;

		DestroyGameSession();
		return;
	}

	CreateSessionCompleteDelegateHandle = SessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	SessionSettings = MakeShareable(new FOnlineSessionSettings());

	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bIsLANMatch = false;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->bUsesPresence = true;
	SessionSettings->NumPublicConnections = NumOfConnections;
	SessionSettings->BuildUniqueId = 1;

	SessionSettings->Set(FName("MatchType"), MatchType, EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	if (!SessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings))
	{
		SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
		MPCreateSessionCompleteDelegate.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::FindGameSession(int32 NumOfSearches)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	FindSessionsCompleteDelegateHandle = SessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	SearchSettings = MakeShareable(new FOnlineSessionSearch());

	SearchSettings->bIsLanQuery = false;
	SearchSettings->MaxSearchResults = NumOfSearches;
	SearchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	if (!SessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SearchSettings.ToSharedRef()))
	{
		SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
		MPFindSessionsCompleteDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Couldn't find sessions"))
			);
		}
	}
}

void UMultiplayerSessionsSubsystem::JoinGameSession(const FOnlineSessionSearchResult& SearchResult)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	SessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);

	ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	if (!SessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, SearchResult))
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
		MPJoinSessionCompleteDelegate.Broadcast(EOnJoinSessionCompleteResult::SessionDoesNotExist, "");
	}
}

void UMultiplayerSessionsSubsystem::StartGameSession()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	StartSessionCompleteDelegateHandle = SessionInterface->AddOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegate);

	if (!SessionInterface->StartSession(NAME_GameSession))
	{
		SessionInterface->ClearOnStartSessionCompleteDelegate_Handle(StartSessionCompleteDelegateHandle);
		MPStartSessionCompleteDelegate.Broadcast(false);
	}
}

void UMultiplayerSessionsSubsystem::DestroyGameSession()
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	DestroySessionCompleteDelegateHandle = SessionInterface->AddOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegate);
	if (!SessionInterface->DestroySession(NAME_GameSession))
	{
		SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
		MPDestroySessionCompleteDelegate.Broadcast(false);
		IsSessionDestroyed = false;
	}
}

void UMultiplayerSessionsSubsystem::OnCreateSessionCompleteDelegate(FName SessionName, bool bWasSuccessful)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	SessionInterface->ClearOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegateHandle);
	MPCreateSessionCompleteDelegate.Broadcast(bWasSuccessful);
}

void UMultiplayerSessionsSubsystem::OnFindSessionsCompleteDelegate(bool bWasSuccessful)
{
	if (!SessionInterface.IsValid())
	{
		return;
	}

	SessionInterface->ClearOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegateHandle);
	if (SearchSettings->SearchResults.Num() < 1)
	{
		MPFindSessionsCompleteDelegate.Broadcast(TArray<FOnlineSessionSearchResult>(), false);
		return;
	}

	if (bWasSuccessful)
	{
		MPFindSessionsCompleteDelegate.Broadcast(SearchSettings->SearchResults, true);
	}
}

void UMultiplayerSessionsSubsystem::OnJoinSessionCompleteDelegate(FName SessionName, EOnJoinSessionCompleteResult::Type SearchResult)
{
	if (!SessionInterface.IsValid())
	{
		MPJoinSessionCompleteDelegate.Broadcast(EOnJoinSessionCompleteResult::UnknownError, "");
		return;
	}

	if (SessionInterface)
	{
		SessionInterface->ClearOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegateHandle);
	}

	FString ConnectInfo;
	if (SessionInterface->GetResolvedConnectString(NAME_GameSession, ConnectInfo))
	{
		MPJoinSessionCompleteDelegate.Broadcast(EOnJoinSessionCompleteResult::Success, ConnectInfo);
	}
}

void UMultiplayerSessionsSubsystem::OnStartSessionCompleteDelegate(FName SessionName, bool bWasSuccessful)
{
}

void UMultiplayerSessionsSubsystem::OnDestroySessionCompleteDelegate(FName SessionName, bool bWasSuccessful)
{
	if (!SessionInterface.IsValid())
	{
		MPDestroySessionCompleteDelegate.Broadcast(false);
		return;
	}

	SessionInterface->ClearOnDestroySessionCompleteDelegate_Handle(DestroySessionCompleteDelegateHandle);
	if (bWasSuccessful && !IsSessionDestroyed)
	{
		IsSessionDestroyed = true;
		CreateGameSession(LastNumOfConnections, LastMatchType);
		MPDestroySessionCompleteDelegate.Broadcast(true);
	}
}
