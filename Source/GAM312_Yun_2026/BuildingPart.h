#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
// Arrow component gives every building part a shared pivot point for rotation and placement
#include "Components/ArrowComponent.h"
// Static mesh component holds the actual wall/floor/ceiling model
#include "Components/StaticMeshComponent.h"
#include "BuildingPart.generated.h"

UCLASS()
class GAM312_YUN_2026_API ABuildingPart : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABuildingPart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Mesh for the building part - set per child blueprint (wall, floor, ceiling)
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;

	// Pivot arrow - the shared start location all building parts rotate and place around
	UPROPERTY(EditAnywhere)
	UArrowComponent* PivotArrow;
};
