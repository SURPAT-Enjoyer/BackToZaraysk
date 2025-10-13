// Fill out your copyright notice in the Description page of Project Settings.


#include "BTZPlayerController.h"
#include"../BTZBaseCharacter.h"
#include "../PlayerCharacter.h"
#include "Blueprint/UserWidget.h"
#include "BackToZaraysk/Inventory/InventoryWidget.h"
#include "BackToZaraysk/Inventory/InventoryItemData.h"
#include "BackToZaraysk/Inventory/InventoryComponent.h"
#include "BackToZaraysk/GameData/Items/Test/PickupBase.h"
#include "BackToZaraysk/GameData/Items/Test/PickupCube.h"
#include "BackToZaraysk/GameData/Items/Test/PickupParallelepiped.h"
#include "BackToZaraysk/GameData/Items/Test/PickupBackpack.h"
#include "BackToZaraysk/GameData/Items/TacticalVest.h"
#include "BackToZaraysk/Inventory/EquippableItemData.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/WidgetBlueprintLibrary.h"
#include "Engine/GameViewportClient.h"
#include "Engine/Engine.h"

ABTZPlayerController::ABTZPlayerController()
{
    // Используем C++ класс напрямую вместо Blueprint
    InventoryWidgetClass = UInventoryWidget::StaticClass();
    
    // Инициализация переменных свободного вращения головы
    bIsFreeLooking = false;
    HeadRotation = FRotator::ZeroRotator;
    InitialHeadRotation = FRotator::ZeroRotator;
    BodyRotation = FRotator::ZeroRotator;
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
    
    // ИСПРАВЛЕНО: Добавлены привязки для системы стрейфа
    InputComponent->BindAction("StrafeLeft", EInputEvent::IE_Pressed, this, &ABTZPlayerController::StartStrafeLeft);
    InputComponent->BindAction("StrafeLeft", EInputEvent::IE_Released, this, &ABTZPlayerController::StopStrafeLeft);
    InputComponent->BindAction("StrafeRight", EInputEvent::IE_Pressed, this, &ABTZPlayerController::StartStrafeRight);
    InputComponent->BindAction("StrafeRight", EInputEvent::IE_Released, this, &ABTZPlayerController::StopStrafeRight);
    InputComponent->BindAction("StrafeSpace", EInputEvent::IE_Pressed, this, &ABTZPlayerController::StartStrafeSpace);
    InputComponent->BindAction("StrafeSpace", EInputEvent::IE_Released, this, &ABTZPlayerController::StopStrafeSpace);
    
    // Свободное вращение камеры средней кнопкой мыши
    InputComponent->BindAction("FreeLook", EInputEvent::IE_Pressed, this, &ABTZPlayerController::StartFreeLook);
    InputComponent->BindAction("FreeLook", EInputEvent::IE_Released, this, &ABTZPlayerController::StopFreeLook);
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
                    // Безопасное создание объекта из класса
                    UObject* NewObj = NewObject<UObject>(this, Pickup->ItemClass);
                    Data = Cast<UInventoryItemData>(NewObj);
                    
                    if (GEngine)
                    {
                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                            FString::Printf(TEXT("🔍 Pickup ItemClass: %s, Created Object: %s"), 
                                *Pickup->ItemClass->GetName(),
                                NewObj ? *NewObj->GetClass()->GetName() : TEXT("null")));
                    }
                    
                    // Единая логика подбора с приоритетами
                    if (Inv->TryPickupItem(Data))
                    {
                        if (InventoryWidgetInstance)
                        {
                            InventoryWidgetInstance->RefreshInventoryUI();
                        }
                        Pickup->Destroy();
                        return;
                    }
                    {
                        FString DataAssetPath = TEXT("/Game/BackToZaraysk/Core/Items/Equipment/DA_TacticalVest.DA_TacticalVest");
                        UObject* LoadedDataAsset = LoadObject<UObject>(nullptr, *DataAssetPath);
                        
                        if (LoadedDataAsset)
                        {
                            if (UEquippableItemData* SourceData = Cast<UEquippableItemData>(LoadedDataAsset))
                            {
                                if (UEquippableItemData* EquipData = Cast<UEquippableItemData>(Data))
                                {
                                    // Копируем все свойства из Data Asset
                                    EquipData->DisplayName = SourceData->DisplayName;
                                    EquipData->SizeInCellsX = SourceData->SizeInCellsX;
                                    EquipData->SizeInCellsY = SourceData->SizeInCellsY;
                                    EquipData->EquipmentSlot = SourceData->EquipmentSlot;
                                    // Перезаписываем сокет на правильный
                                    EquipData->AttachSocketName = FName(TEXT("spine_02"));
                                    EquipData->EquippedMesh = SourceData->EquippedMesh;
                                    // Перезаписываем трансформ на правильный
                                    EquipData->RelativeTransform = FTransform(
                                        FRotator(0.0f, 0.0f, 0.0f),
                                        FVector(5.0f, 0.0f, -2.0f),
                                        FVector(1.0f, 1.0f, 1.0f)
                                    );
                                    EquipData->bRotatable = SourceData->bRotatable;
                                    
                                    if (GEngine)
                                    {
                                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                                            FString::Printf(TEXT("🔧 Data Asset properties copied! EquippedMesh: %s"), 
                                                EquipData->EquippedMesh ? TEXT("SET") : TEXT("NULL")));
                                    }
                                }
                            }
                        }
                        else
                        {
                            if (GEngine)
                            {
                                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                                    TEXT("❌ Failed to load Data Asset from path!"));
                            }
                        }
                    }
                    
                    if (GEngine)
                    {
                        // Проверяем, является ли это экипируемым предметом
                        if (UEquippableItemData* EquipData = Cast<UEquippableItemData>(Data))
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                                FString::Printf(TEXT("✅ EquippableItemData found! EquippedMesh: %s, Slot: %d"), 
                                    EquipData->EquippedMesh ? TEXT("SET") : TEXT("NULL"),
                                    (int32)EquipData->EquipmentSlot));
                        }
                        else if (Data)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                                FString::Printf(TEXT("⚠️ Created object is not UEquippableItemData, it's: %s"), 
                                    *Data->GetClass()->GetName()));
                        }
                    }
                }
                else
                {
                    // Если ItemClass не установлен, создаем предмет на основе типа Pickup
                    if (Cast<ATacticalVest>(Pickup))
                    {
                        // Пытаемся загрузить Data Asset для тактического жилета
                        FString DataAssetPath = TEXT("/Game/BackToZaraysk/Core/Items/Equipment/DA_TacticalVest.DA_TacticalVest");
                        UObject* LoadedDataAsset = LoadObject<UObject>(nullptr, *DataAssetPath);
                        
                        if (LoadedDataAsset)
                        {
                            // Создаем новый экземпляр на основе загруженного Data Asset
                            UObject* NewObj = NewObject<UObject>(this, LoadedDataAsset->GetClass());
                            Data = Cast<UInventoryItemData>(NewObj);
                            
                            if (UEquippableItemData* SourceData = Cast<UEquippableItemData>(LoadedDataAsset))
                            {
                                if (UEquippableItemData* EquipData = Cast<UEquippableItemData>(Data))
                                {
                                    // Копируем все свойства из Data Asset
                                    EquipData->DisplayName = SourceData->DisplayName;
                                    EquipData->SizeInCellsX = SourceData->SizeInCellsX;
                                    EquipData->SizeInCellsY = SourceData->SizeInCellsY;
                                    EquipData->EquipmentSlot = SourceData->EquipmentSlot;
                                    // Перезаписываем сокет на правильный
                                    EquipData->AttachSocketName = FName(TEXT("spine_02"));
                                    EquipData->EquippedMesh = SourceData->EquippedMesh;
                                    // Перезаписываем трансформ на правильный
                                    EquipData->RelativeTransform = FTransform(
                                        FRotator(0.0f, 0.0f, 0.0f),
                                        FVector(5.0f, 0.0f, -2.0f),
                                        FVector(1.0f, 1.0f, 1.0f)
                                    );
                                    EquipData->bRotatable = SourceData->bRotatable;
                                    
                                    if (GEngine)
                                    {
                                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                                            FString::Printf(TEXT("🔧 Loaded Data Asset: %s, EquippedMesh: %s, Slot: %d"), 
                                                *EquipData->DisplayName.ToString(),
                                                EquipData->EquippedMesh ? TEXT("SET") : TEXT("NULL"),
                                                (int32)EquipData->EquipmentSlot));
                                        
                                        // Дополнительная диагностика
                                        if (EquipData->EquippedMesh)
                                        {
                                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                                                FString::Printf(TEXT("✅ EquippedMesh name: %s"), 
                                                    *EquipData->EquippedMesh->GetName()));
                                        }
                                        else
                                        {
                                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                                                TEXT("❌ EquippedMesh is NULL in Data Asset!"));
                                        }
                                    }
                                }
                            }
                        }
                        else
                        {
                            // Fallback: создаем UEquippableItemData вручную
                            Data = NewObject<UEquippableItemData>(this);
                            if (UEquippableItemData* EquipData = Cast<UEquippableItemData>(Data))
                            {
                                EquipData->DisplayName = FText::FromString(TEXT("Тактический жилет"));
                                EquipData->SizeInCellsX = 3;
                                EquipData->SizeInCellsY = 3;
                                EquipData->EquipmentSlot = Vest;
                                EquipData->AttachSocketName = FName(TEXT("spine_02"));
                                EquipData->bRotatable = false;
                                
                                // Попытаемся загрузить меш вручную
                                FString MeshPath = TEXT("/Game/insurgent_2/Characters/SK_ChestRigSmall.SK_ChestRigSmall");
                                USkeletalMesh* VestMesh = LoadObject<USkeletalMesh>(nullptr, *MeshPath);
                                if (VestMesh)
                                {
                                    EquipData->EquippedMesh = VestMesh;
                                    if (GEngine)
                                    {
                                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Green, 
                                            FString::Printf(TEXT("✅ Manually loaded EquippedMesh: %s"), 
                                                *VestMesh->GetName()));
                                    }
                                }
                                else
                                {
                                    if (GEngine)
                                    {
                                        GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Red, 
                                            FString::Printf(TEXT("❌ Failed to load mesh from path: %s"), *MeshPath));
                                    }
                                }
                            }
                            if (GEngine)
                            {
                                GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Yellow, 
                                    TEXT("⚠️ Data Asset not found, created UEquippableItemData manually"));
                            }
                        }
                    }
                    else
                    {
                        Data = NewObject<UInventoryItemData>(this);
                        if (GEngine)
                        {
                            GEngine->AddOnScreenDebugMessage(-1, 3.0f, FColor::Cyan, 
                                TEXT("🔍 Pickup ItemClass is null, created default UInventoryItemData"));
                        }
                    }
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
    
    // Получаем последний предмет из инвентаря
    if (Inv->BackpackItems.Num() == 0) return;
    UInventoryItemData* Item = Inv->BackpackItems.Last();
    if (!Item) return;
    
    // Удаляем предмет из инвентаря
    Inv->RemoveSpecificFromBackpack(Item);
    
    FVector ViewLoc; FRotator ViewRot; GetPlayerViewPoint(ViewLoc, ViewRot);
    const FVector SpawnLoc = ViewLoc + ViewRot.Vector() * 80.f;
    FActorSpawnParameters S; S.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;
    
    TSubclassOf<AActor> DropClass = APickupCube::StaticClass();
    
    // Проверяем, является ли предмет экипируемым
    if (UEquippableItemData* EquippableItem = Cast<UEquippableItemData>(Item))
    {
        if (EquippableItem->SizeInCellsX == 3 && EquippableItem->SizeInCellsY == 3)
        {
            // Для тактического жилета 3x3
            DropClass = ATacticalVest::StaticClass();
        }
    }
    else if (Item->SizeInCellsX == 2 && Item->SizeInCellsY == 1)
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
	if (bIsFreeLooking) 
	{
		// FreeLook имеет приоритет - обрабатываем как свободное вращение камеры
		FreeLookX(Value);
		return;
	}
	
	// Обычное вращение персонажа
	if (ensure(CachedBaseCharacter.IsValid()))
	{
		CachedBaseCharacter->Turn(Value);
	}
}

void ABTZPlayerController::LookUp(float Value)
{
	if (bIsFreeLooking) 
	{
		// FreeLook имеет приоритет - обрабатываем как свободное вращение камеры
		FreeLookY(Value);
		return;
	}
	
	// Обычное вращение камеры
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
            // Принудительный рефреш UI инвентаря
            if (CachedBaseCharacter.IsValid())
            {
                if (UInventoryComponent* Inv = CachedBaseCharacter->FindComponentByClass<UInventoryComponent>())
                {
                    InventoryWidgetInstance->RefreshInventoryUI();
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

// ИСПРАВЛЕНО: Добавлены функции стрейфа в PlayerController
void ABTZPlayerController::StartStrafeLeft()
{
    if (ensure(CachedBaseCharacter.IsValid()))
    {
        if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
        {
            PC->HandleAInput(true);
        }
    }
}

void ABTZPlayerController::StopStrafeLeft()
{
    if (ensure(CachedBaseCharacter.IsValid()))
    {
        if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
        {
            PC->HandleAInput(false);
        }
    }
}

void ABTZPlayerController::StartStrafeRight()
{
    if (ensure(CachedBaseCharacter.IsValid()))
    {
        if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
        {
            PC->HandleDInput(true);
        }
    }
}

void ABTZPlayerController::StopStrafeRight()
{
    if (ensure(CachedBaseCharacter.IsValid()))
    {
        if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
        {
            PC->HandleDInput(false);
        }
    }
}

void ABTZPlayerController::StartStrafeSpace()
{
    if (ensure(CachedBaseCharacter.IsValid()))
    {
        if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
        {
            PC->HandleSpaceInput(true);
        }
    }
}

void ABTZPlayerController::StopStrafeSpace()
{
    if (ensure(CachedBaseCharacter.IsValid()))
    {
        if (APlayerCharacter* PC = Cast<APlayerCharacter>(CachedBaseCharacter.Get()))
        {
            PC->HandleSpaceInput(false);
        }
    }
}

// Свободное вращение камеры средней кнопкой мыши
void ABTZPlayerController::StartFreeLook()
{
    if (bIsFreeLooking) return;
    
    bIsFreeLooking = true;
    
    // Инициализируем поворот головы
    if (CachedBaseCharacter.IsValid())
    {
        // Запоминаем текущую ротацию камеры как базовую
        BodyRotation = GetControlRotation();
        
        // Сбрасываем поворот головы
        CachedBaseCharacter->ResetHeadRotation();
        
        // Блокируем поворот тела
        CachedBaseCharacter->SetRotationBlocked(true);
        
        // Отключаем поворот персонажа на уровне движения
        if (UCharacterMovementComponent* CharMovement = CachedBaseCharacter->GetCharacterMovement())
        {
            CharMovement->bOrientRotationToMovement = false;
            CharMovement->bUseControllerDesiredRotation = false;
        }
    }
    
    // Инициализируем переменные поворота головы
    HeadRotation = FRotator::ZeroRotator;
    InitialHeadRotation = FRotator::ZeroRotator;
    
    // Блокируем стандартное управление камерой
    SetIgnoreLookInput(true);
    SetIgnoreMoveInput(false); // Движение разрешено
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("FreeLook: Started - Head rotation only"));
    }
}

void ABTZPlayerController::StopFreeLook()
{
    if (!bIsFreeLooking) return;
    
    bIsFreeLooking = false;
    
    // Восстанавливаем стандартное управление
    SetIgnoreLookInput(false);
    
    // Снимаем блокировку на уровне персонажа
    if (CachedBaseCharacter.IsValid())
    {
        CachedBaseCharacter->SetRotationBlocked(false);
        
        // Сбрасываем поворот головы
        CachedBaseCharacter->ResetHeadRotation();
        
        // Восстанавливаем настройки движения персонажа
        if (UCharacterMovementComponent* CharMovement = CachedBaseCharacter->GetCharacterMovement())
        {
            CharMovement->bOrientRotationToMovement = true;
            CharMovement->bUseControllerDesiredRotation = true;
        }
    }
    
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Cyan, TEXT("FreeLook: Stopped - Head reset to body"));
    }
}

void ABTZPlayerController::FreeLookX(float Value)
{
    if (!bIsFreeLooking) return;
    
    // Горизонтальное вращение головы для Transform (Modify) Bone (в градусах)
    HeadRotation.Yaw += Value * 2.0f; // Чувствительность поворота головы
    
    // Ограничиваем поворот головы влево/вправо (±120 градусов)
    HeadRotation.Yaw = FMath::Clamp(HeadRotation.Yaw, -120.0f, 120.0f);
    
    // Применяем поворот головы через Transform (Modify) Bone
    if (CachedBaseCharacter.IsValid())
    {
        CachedBaseCharacter->SetHeadRotation(HeadRotation.Yaw, HeadRotation.Pitch);
    }
    
    // Обновляем камеру - она должна следовать за головой
    // Поворачиваем камеру на HeadRotation относительно BodyRotation
    FRotator CombinedRotation = FRotator(
        BodyRotation.Pitch + HeadRotation.Pitch,
        BodyRotation.Yaw + HeadRotation.Yaw,
        BodyRotation.Roll + HeadRotation.Roll
    );
    SetControlRotation(CombinedRotation);
}

void ABTZPlayerController::FreeLookY(float Value)
{
    if (!bIsFreeLooking) return;
    
    // Вертикальное вращение головы для Transform (Modify) Bone (в градусах)
    HeadRotation.Pitch -= Value * 2.0f; // Чувствительность наклона головы
    
    // Ограничиваем наклон головы вверх/вниз (±60 градусов)
    HeadRotation.Pitch = FMath::Clamp(HeadRotation.Pitch, -60.0f, 60.0f);
    
    // Применяем поворот головы через Transform (Modify) Bone
    if (CachedBaseCharacter.IsValid())
    {
        CachedBaseCharacter->SetHeadRotation(HeadRotation.Yaw, HeadRotation.Pitch);
    }
    
    // Обновляем камеру - она должна следовать за головой
    // Поворачиваем камеру на HeadRotation относительно BodyRotation
    FRotator CombinedRotation = FRotator(
        BodyRotation.Pitch + HeadRotation.Pitch,
        BodyRotation.Yaw + HeadRotation.Yaw,
        BodyRotation.Roll + HeadRotation.Roll
    );
    SetControlRotation(CombinedRotation);
}
