// Fill out your copyright notice in the Description page of Project Settings.

#include "ShooterHUD.h"
#include "CharacterOverlay.h"

void AShooterHUD::BeginPlay()
{
    Super::BeginPlay();

    AddCharacterOverlay();
}

void AShooterHUD::DrawHUD()
{
    Super::DrawHUD();

    if (GEngine)
    {
        FVector2D ViewportSize;
        GEngine->GameViewport->GetViewportSize(ViewportSize);
        FVector2D ViewportCenter(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);

        HUDPackage.SpreadFactor *= CrosshairMultiplier;

        if (HUDPackage.CrosshairCenter)
        {
            FVector2D SpreadFactor(0.f, 0.f);
            DrawCrosshairToScreen(HUDPackage.CrosshairCenter, ViewportCenter, SpreadFactor, HUDPackage.CrosshairColor);
        }
        if (HUDPackage.CrosshairTop)
        {
            FVector2D SpreadFactor(0.f, -HUDPackage.SpreadFactor);
            DrawCrosshairToScreen(HUDPackage.CrosshairTop, ViewportCenter, SpreadFactor, HUDPackage.CrosshairColor);
        }
        if (HUDPackage.CrosshairBottom)
        {
            FVector2D SpreadFactor(0.f, HUDPackage.SpreadFactor);
            DrawCrosshairToScreen(HUDPackage.CrosshairBottom, ViewportCenter, SpreadFactor, HUDPackage.CrosshairColor);
        }
        if (HUDPackage.CrosshairLeft)
        {
            FVector2D SpreadFactor(-HUDPackage.SpreadFactor, 0.f);
            DrawCrosshairToScreen(HUDPackage.CrosshairLeft, ViewportCenter, SpreadFactor, HUDPackage.CrosshairColor);
        }
        if (HUDPackage.CrosshairRight)
        {
            FVector2D SpreadFactor(HUDPackage.SpreadFactor, 0.f);
            DrawCrosshairToScreen(HUDPackage.CrosshairRight, ViewportCenter, SpreadFactor, HUDPackage.CrosshairColor);
        }
    }
}

void AShooterHUD::DrawCrosshairToScreen(UTexture2D *CrosshairTexture,
                                        FVector2D ViewportCenter, FVector2D SpreadFactor, FLinearColor CrosshairColor)
{
    float TextureWidth = CrosshairTexture->GetSizeX();
    float TextureHeight = CrosshairTexture->GetSizeY();

    FVector2D CrosshairAxis(
        ViewportCenter.X - (TextureWidth / 2.f) + SpreadFactor.X,
        ViewportCenter.Y - (TextureHeight / 2.f) + SpreadFactor.Y);

    DrawTexture(CrosshairTexture,
                CrosshairAxis.X,
                CrosshairAxis.Y,
                TextureWidth,
                TextureHeight,
                0.f, 0.f,
                1.f, 1.f,
                CrosshairColor);
}

void AShooterHUD::AddCharacterOverlay()
{
    APlayerController *PlayerController = GetOwningPlayerController();
    if (PlayerController && CharacterOverlayClass)
    {
        CharacterOverlay = CreateWidget<UCharacterOverlay>(PlayerController, CharacterOverlayClass);
        CharacterOverlay->AddToViewport();
    }
}