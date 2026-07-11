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

	// Name of this resource - editable per instance/child so Wood, Stone, and Berry can share this master class
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
