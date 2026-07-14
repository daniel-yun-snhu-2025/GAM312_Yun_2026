#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
// TextRender is used to display the resource name above the mesh for testing purposes
#include "Components/TextRenderComponent.h"
// StaticMeshComponent lets us give the resource an actual mesh in the world
#include "Components/StaticMeshComponent.h"
#include "Resource_M.generated.h"

UCLASS()
class GAM312_YUN_2026_API AResource_M : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AResource_M();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Runs in the editor whenever the actor is placed or a property changes,
	// so the object name label updates live in the viewport (not just at runtime)
	virtual void OnConstruction(const FTransform& Transform) override;

	// Display name of this interactable object (Tree, Rock, Bush) - shown on the label above the mesh
	UPROPERTY(EditAnywhere)
	FString objectName = "Tree";

	// Name of the resource this object yields when harvested (Wood, Stone, Berry) -
	// drives which inventory slot the player character increments
	UPROPERTY(EditAnywhere)
	FString resourceName = "Wood";

	// How much this resource gives the player each time it is hit
	UPROPERTY(EditAnywhere)
	int resourceAmount = 5;

	// Total amount available before the resource is depleted
	UPROPERTY(EditAnywhere)
	int totalResource = 100;

	// Holds the resource name converted to text for the TextRender component
	UPROPERTY()
	FText tempText;

	// Text render that displays the resource name in the world (testing aid)
	UPROPERTY(EditAnywhere)
	UTextRenderComponent* ResourceNameText;

	// Mesh for the resource - set on the Blueprint side
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* Mesh;
};
