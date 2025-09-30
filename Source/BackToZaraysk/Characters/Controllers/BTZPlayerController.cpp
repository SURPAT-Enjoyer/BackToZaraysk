// Fill out your copyright notice in the Description page of Project Settings.


#include "BTZPlayerController.h"
#include"../BTZBaseCharacter.h"
#include "../PlayerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "BackToZaraysk/Inventory/InventoryWidget.h"
#include "BackToZaraysk/GameData/Items/Test/PickupBase.h"
#include "BackToZaraysk/GameData/Items/Test/PickupCube.h"
#include "BackToZaraysk/GameData/Items/Test/PickupParallelepiped.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"

ABTZPlayerController::ABTZPlayerController()
{
    // Используем C++ класс напрямую вместо Blueprint
    InventoryWidgetClass = UInventoryWidget::StaticClass();
}

void ABTZPlayerController::RequestToggleInventory()
{
    ToggleInventory();
}

void ABTZPlayerController::SetPawn(APawn* InPawn)
{
	Super::SetPawn(InPawn);
	CachedBaseCharacter = Cast<ABTZBaseCharacter>(InPawn);
}

void ABTZPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	InputComponent->BindAxis("MoveForward", this, &ABTZPlayerController::MoveForward);
	InputComponent->BindAxis("MoveRight", this, &ABTZPlayerController::MoveRight);
	InputComponent->BindAxis("Turn", this, &ABTZPlayerController::Turn);
	InputComponent->BindAxis("LookUp", this, &ABTZPlayerController::LookUp);
	InputComponent->BindAction("Jump", EInputEvent::IE_Pressed, this, &ABTZPlayerController::Jump);
	InputComponent->BindAction("Crouch", EInputEvent::IE_Pressed, this, &ABTZPlayerController::ChangeCrouchState);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Pressed, this, &ABTZPlayerController::StartSprint);
	InputComponent->BindAction("Sprint", EInputEvent::IE_Released, this, &ABTZPlayerController::StopSprint);
	InputComponent->BindAction("Prone", EInputEvent::IE_Pressed, this, &ABTZPlayerController::ChangeProneState);
    InputComponent->BindAction("OpenInventory", EInputEvent::IE_Pressed, this, &ABTZPlayerController::ToggleInventory);
    InputComponent->BindAction("Interact", EInputEvent::IE_Pressed, this, &ABTZPlayerController::Interact);
    InputComponent->BindAction("Drop", EInputEvent::IE_Pressed, this, &ABTZPlayerController::DropItem);
    InputComponent->BindAction("ClimbObstacle", EInputEvent::IE_Pressed, this, &ABTZPlayerController::TryClimbObstacle);
    // Lean
    InputComponent->BindAction("LeanLeft", EInputEvent::IE_Pressed, this, &ABTZPlayerController::StartLeanLeft);
    InputComponent->BindAction("LeanLeft", EInputEvent::IE_Released, this, &ABTZPlayerController::StopLeanLeft);
    InputComponent->BindAction("LeanRight", EInputEvent::IE_Pressed, this, &ABTZPlayerController::StartLeanRight);
    InputComponent->BindAction("LeanRight", EInputEvent::IE_Released, this, &ABTZPlayerController::StopLeanRight);
}
void ABTZPlayerController::Interact()
{
    if (!CachedBaseCharacter.IsValid()) return;
    FVector ViewLoc; FRotator ViewRot; GetPlayerViewPoint(ViewLoc, ViewRot);
    const FVector Start = ViewLoc;
    const FVector End = Start + ViewRot.Vector() * 200.f;
    FHitResult Hit;
    FCollisionQueryParams Params(SCENE_QUERY_STAT(InteractTrace), false);
    if (GetWorld()->LineTraceSingleByChannel(Hit, Start, End, ECC_Visibility, Params))
    {
        class APickupBase* Pickup = Cast<class APickupBase>(Hit.GetActor());
        if (Pickup)
        {
            UInventoryComponent* Inv = CachedBaseCharacter->FindComponentByClass<UInventoryComponent>();
            if (Inv)
            {
                UInventoryItemData* Data = nullptr;
                if (Pickup->ItemClass)
                {
                    Data = NewObject<UInventoryItemData>(this, Pickup->ItemClass);
                }
                else
                {
                    Data = NewObject<UInventoryItemData>(this);
                }
                if (Data && Data->SizeInCellsX<=0) { Data->SizeInCellsX=1; }
                if (Data && Data->SizeInCellsY<=0) { Data->SizeInCellsY=1; }
                if (!Data->Icon)
                {
                    // Иконка: белый квадрат по умолчанию
                    if (UTexture2D* White = LoadObject<UTexture2D>(nullptr, TEXT("/Engine/EngineResources/WhiteSquareTexture.WhiteSquareTexture")))
                    {
                        Data->Icon = White;
                    }
                }
                if (Inv->AddToBackpack(Data))
                {
                    Pickup->Destroy();
                    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Green, TEXT("Picked up to backpack"));
                    if (InventoryWidgetInstance)
                    {
                        InventoryWidgetInstance->AddBackpackItemIcon(Data);
                    }
                }
            }
        }
    }
}

void ABTZPlayerController::DropItem()
{
    if (!CachedBaseCharacter.IsValid()) return;
    UInventoryComponent* Inv = CachedBaseCharacter->FindComponentByClass<UInventoryComponent>();
    if (!Inv) return;
    UInventoryItemData* Item = Inv->RemoveLastFromBackpack();
    if (!Item) return;
    FVector ViewLoc; FRotator ViewRot; GetPlayerViewPoint(ViewLoc, ViewRot);
    const FVector SpawnLoc = ViewLoc + ViewRot.Vector() * 80.f;
    FActorSpawnParameters S; S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    TSubclassOf<AActor> DropClass = APickupCube::StaticClass();
    if (Item->SizeInCellsX == 2 && Item->SizeInCellsY == 1)
    {
        DropClass = APickupParallelepiped::StaticClass();
    }
    AActor* Spawned = GetWorld()->SpawnActor<AActor>(DropClass, SpawnLoc, ViewRot, S);
    if (GEngine) GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("Dropped from backpack"));
    if (InventoryWidgetInstance && CachedBaseCharacter.IsValid())
    {
        if (UInventoryComponent* Inv2 = CachedBaseCharacter->FindComponentByClass<UInventoryComponent>())
        {
            InventoryWidgetInstance->SyncBackpack(Inv2->BackpackItems);
        }
    }
}

void ABTZPlayerController::MoveForward(float Value)
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->MoveForward(Value);
	}
}

void ABTZPlayerController::MoveRight(float Value)
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->MoveRight(Value);
	}
}

void ABTZPlayerController::Turn(float Value)
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->Turn(Value);
	}
}

void ABTZPlayerController::LookUp(float Value)
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->LookUp(Value);
	}
}

void ABTZPlayerController::Jump()
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
        CachedBaseCharacter->Jump();
	}
}
void ABTZPlayerController::ChangeCrouchState()
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->ChangeCrouchState();
	}

}

void ABTZPlayerController::StartSprint()
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->StartSprint();
	}
}

void ABTZPlayerController::StopSprint()
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->StopSprint();
	}
}

void ABTZPlayerController::ChangeProneState()
{
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->ChangeProneState();
	}
}

void ABTZPlayerController::ToggleInventory()
{
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("ToggleInventory pressed"));
    }
    if (!InventoryWidgetInstance)
    {
        TSubclassOf<UInventoryWidget> ClassToUse = InventoryWidgetClass;
        if (!ClassToUse)
        {
            ClassToUse = UInventoryWidget::StaticClass();
        }
        InventoryWidgetInstance = CreateWidget<UInventoryWidget>(this, ClassToUse);
        if (InventoryWidgetInstance)
        {
            // Стандартный способ UMG — AddToViewport (устойчиво для PIE)
            InventoryWidgetInstance->AddToViewport(10000);
            InventoryWidgetInstance->SetVisible(false);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Inventory widget created"));
            }
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("ToggleInventory: Failed to create inventory widget"));
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Failed to create inventory widget"));
            }
        }
    }

    if (InventoryWidgetInstance)
    {
        const bool bShow = !InventoryWidgetInstance->IsInventoryVisible();
        InventoryWidgetInstance->SetVisible(bShow);
        if (bShow)
        {
            // Режим Игра+UI: клавиша I, Ctrl, Shift доходят до контроллера; блокируем только Look
            FInputModeGameAndUI Mode;
            Mode.SetWidgetToFocus(InventoryWidgetInstance->TakeWidget());
            Mode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
            Mode.SetHideCursorDuringCapture(false);
            SetInputMode(Mode);
            SetIgnoreLookInput(true);
            SetShowMouseCursor(true);
            // Гарантируем, что во вьюпорте включён показ UI в PIE
            ConsoleCommand(TEXT("showflag.ui 1"), true);
            // Синхронизация содержимого рюкзака в UI
            if (CachedBaseCharacter.IsValid())
            {
                if (UInventoryComponent* Inv = CachedBaseCharacter->FindComponentByClass<UInventoryComponent>())
                {
                    InventoryWidgetInstance->SyncBackpack(Inv->BackpackItems);
                }
            }
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Inventory OPENED"));
            }
        }
        else
        {
            UWidgetBlueprintLibrary::SetInputMode_GameOnly(this);
            SetIgnoreLookInput(false);
            SetShowMouseCursor(false);
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("Inventory CLOSED"));
            }
        }
    }
}

void ABTZPlayerController::TryClimbObstacle()
{
    if (ensure(CachedBaseCharacter.IsValid()))
    {
        // Проверяем, что персонаж движется вперед (нажата W)
        float VelocitySize = CachedBaseCharacter->GetVelocity().Size();
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 1.0f, FColor::White, 
                FString::Printf(TEXT("Climb Input: Velocity = %.2f"), VelocitySize));
        }
        
        if (VelocitySize > 0.0f)
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("Climb Input: Attempting obstacle climb..."));
            }
            // Пытаемся преодолеть препятствие
            CachedBaseCharacter->TryClimbObstacle();
        }
        else
        {
            if (GEngine)
            {
                GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Orange, TEXT("Climb Input: Character not moving - climb cancelled"));
            }
        }
    }
    else
    {
        if (GEngine)
        {
            GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Climb Input: No valid character!"));
        }
    }
}

void ABTZPlayerController::StartLeanLeft()
{
    if (ensure(CachedBaseCharacter.IsValid()))
    {
        if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
        {
            PC->SetLeanTarget(-PC->MaxLeanAngleDeg);
        }
    }
}

void ABTZPlayerController::StopLeanLeft()
{
    if (ensure(CachedBaseCharacter.IsValid()))
    {
        if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
        {
            PC->ResetLean();
        }
    }
}

void ABTZPlayerController::StartLeanRight()
{
    if (ensure(CachedBaseCharacter.IsValid()))
    {
        if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
        {
            PC->SetLeanTarget(PC->MaxLeanAngleDeg);
        }
    }
}

void ABTZPlayerController::StopLeanRight()
{
    if (ensure(CachedBaseCharacter.IsValid()))
    {
        if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
        {
            PC->ResetLean();
        }
    }
}
