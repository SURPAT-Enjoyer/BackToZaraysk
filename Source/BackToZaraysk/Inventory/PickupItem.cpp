#include "PickupItem.h"
#include "Components/StaticMeshComponent.h"
#include "UObject/ConstructorHelpers.h"
#include "Engine/StaticMesh.h"
#include "InventoryItemData.h"
#include "Materials/MaterialInterface.h"

APickupItem::APickupItem()
{
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	SetRootComponent(MeshComponent);
	MeshComponent->SetSimulatePhysics(true);
	MeshComponent->SetCollisionProfileName(TEXT("PhysicsActor"));

	// Куб по умолчанию
	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMesh(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMesh.Succeeded())
	{
		MeshComponent->SetStaticMesh(CubeMesh.Object);
		MeshComponent->SetWorldScale3D(FVector(0.3f));
	}

    // Белая простая текстура/материал по умолчанию
    static ConstructorHelpers::FObjectFinder<UMaterialInterface> WhiteMat(TEXT("/Engine/EngineMaterials/DefaultWhiteGrid.DefaultWhiteGrid"));
    if (WhiteMat.Succeeded())
    {
        MeshComponent->SetMaterial(0, WhiteMat.Object);
    }
}


