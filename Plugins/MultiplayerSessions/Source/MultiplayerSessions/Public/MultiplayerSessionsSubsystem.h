// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "MultiplayerSessionsSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMPCreateSessionCompleteDelegate, bool, bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMPFindSessionsCompleteDelegate, const TArray<FOnlineSessionSearchResult>& SearchResults, bool bWasSuccessful);
DECLARE_MULTICAST_DELEGATE_TwoParams(FMPJoinSessionCompleteDelegate, EOnJoinSessionCompleteResult::Type Result, FString ConnectString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMPStartSessionCompleteDelegate, bool, bWasSuccessful);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FMPDestroySessionCompleteDelegate, bool, bWasSuccessful);

UCLASS()
class MULTIPLAYERSESSIONS_API UMultiplayerSessionsSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UMultiplayerSessionsSubsystem();

	void CreateGameSession(int32 NumOfConnections, FString MatchType);
	void FindGameSession(int32 NumOfSearches);
	void JoinGameSession(const FOnlineSessionSearchResult& SearchResult);
	void StartGameSession();
	void DestroyGameSession();

	FMPCreateSessionCompleteDelegate MPCreateSessionCompleteDelegate;
	FMPFindSessionsCompleteDelegate MPFindSessionsCompleteDelegate;
	FMPJoinSessionCompleteDelegate MPJoinSessionCompleteDelegate;
	FMPStartSessionCompleteDelegate MPStartSessionCompleteDelegate;
	FMPDestroySessionCompleteDelegate MPDestroySessionCompleteDelegate;

protected:
	void OnCreateSessionCompleteDelegate(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsCompleteDelegate(bool bWasSuccessful);
	void OnJoinSessionCompleteDelegate(FName SessionName, EOnJoinSessionCompleteResult::Type SearchResult);
	void OnStartSessionCompleteDelegate(FName SessionName, bool bWasSuccessful);
	void OnDestroySessionCompleteDelegate(FName SessionName, bool bWasSuccessful);

private:
	bool IsSessionDestroyed{ false };
	int32 LastNumOfConnections{ 0 };
	FString LastMatchType{ TEXT("") };

	IOnlineSessionPtr SessionInterface;
	TSharedPtr<FOnlineSessionSettings> SessionSettings;
	TSharedPtr<FOnlineSessionSearch> SearchSettings;

	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	FOnStartSessionCompleteDelegate StartSessionCompleteDelegate;
	FOnDestroySessionCompleteDelegate DestroySessionCompleteDelegate;

	FDelegateHandle CreateSessionCompleteDelegateHandle;
	FDelegateHandle FindSessionsCompleteDelegateHandle;
	FDelegateHandle JoinSessionCompleteDelegateHandle;
	FDelegateHandle StartSessionCompleteDelegateHandle;
	FDelegateHandle DestroySessionCompleteDelegateHandle;
};
