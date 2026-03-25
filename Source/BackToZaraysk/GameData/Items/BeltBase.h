#pragma once

#include "CoreMinimal.h"
#include "EquipmentBase.h"
#include "BeltBase.generated.h"

class UTexture2D;

UCLASS(Blueprintable)
class BACKTOZARAYSK_API ABeltBase : public AEquipmentBase
{
    GENERATED_BODY()

public:
    ABeltBase();

protected:
    virtual void BeginPlay() override;
    virtual void OnConstruction(const FTransform& Transform) override;

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Belt|Inventory", meta=(DisplayName="Inventory Icon Texture"))
    TObjectPtr<UTexture2D> InventoryIconTexture = nullptr;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Belt|Inventory", meta=(ClampMin="1", UIMin="1"))
    int32 InventorySizeX = 2;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Belt|Inventory", meta=(ClampMin="1", UIMin="1"))
    int32 InventorySizeY = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Belt|Storage", meta=(ClampMin="1", UIMin="1"))
    int32 StorageGridCount = 1;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Belt|Storage")
    TArray<FIntPoint> StorageGridSizes;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Belt|Storage")
    TArray<FIntPoint> StorageGridOffsets;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Belt|Attachment", meta=(DisplayName="Attach Bone/Socket"))
    FName AttachBoneName = FName(TEXT("spine_01"));

private:
    void EnsureStorageGrids();
    void ApplyEditorOverridesToItemInstance();
    void ApplyInventoryIconOverride();
};

