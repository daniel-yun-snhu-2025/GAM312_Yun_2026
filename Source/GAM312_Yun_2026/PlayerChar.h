#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
// Included so we can cast line trace hits to our master resource class
#include "Resource_M.h"
// Included for SpawnDecalAtLocation when the line trace hits
#include "Kismet/GameplayStatics.h"
// Included so the player can spawn and manage building parts (wall, floor, ceiling)
#include "BuildingPart.h"
// Included so the player can create and update the on-screen stat bar widget
#include "PlayerWidget.h"
// Included so the player can create and update the objective tracker widget
#include "ObjectiveWidget.h"
#include "PlayerChar.generated.h"

class UCameraComponent;

UCLASS()
class GAM312_YUN_2026_API APlayerChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerChar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Establishing the functions that we are going to be utilizing for the character's movement
	// Forward and backward movement, receives the axis value from the MoveForward input (+1 = W, -1 = S)
	UFUNCTION()
	void MoveForward(float axisValue);

	// Left and right movement, receives the axis value from the MoveRight input (-1 = A, +1 = D)
	UFUNCTION()
	void MoveRight(float axisValue);

	// Custom jump events - set and clear the jump flag used by the built-in Character jump
	UFUNCTION()
	void StartJump();

	UFUNCTION()
	void StopJump();

	// FindObject event - will run the line trace for detecting objects when Interact is pressed
	UFUNCTION()
	void FindObject();

	// Establishing the camera component so we can access the camera in code and attach it to the head
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* PlayerCamComp;

	// Player stat variables - editable in the Details panel and readable/writable from Blueprints,
	// grouped under the Player Stats category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Hunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina = 100.0f;

	// Blueprint Callable events for adjusting the player stats - a positive amount adds to
	// the stat and a negative amount subtracts from it
	UFUNCTION(BlueprintCallable)
	void SetHealth(float amount);

	UFUNCTION(BlueprintCallable)
	void SetHunger(float amount);

	UFUNCTION(BlueprintCallable)
	void SetStamina(float amount);

	// Called on a timer - decreases Hunger over time, regenerates Stamina,
	// and drains Health once Hunger reaches zero
	UFUNCTION()
	void DecreaseStats();

	// Adds the collected amount to the matching index of the resources array
	UFUNCTION()
	void GiveResource(float amount, FString resourceType);

	// Timer handle driving the DecreaseStats interval
	FTimerHandle StatsTimerHandle;

	// Individual resource counts, grouped under the Resources category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	int Wood;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	int Stone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	int Berry;

	// Overall resources array - index 0 is Wood, 1 is Stone, 2 is Berry
	UPROPERTY(EditAnywhere, Category = "Resources")
	TArray<int> ResourcesArray;

	// Names matching the resources array indices - these must line up with ResourcesArray
	UPROPERTY(EditAnywhere, Category = "Resources")
	TArray<FString> ResourcesNameArray;

	// Decal material spawned at the line trace hit location as a visual indicator
	UPROPERTY(EditAnywhere, Category = "HitMarker")
	UMaterialInterface* hitDecal;

	// How many of each building part the player has crafted - index 0 is Wall, 1 is Floor, 2 is Ceiling
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Building Supplies")
	TArray<int> BuildingArray;

	// True while the player is actively placing a building part
	UPROPERTY()
	bool isBuilding;

	// Which building part class to spawn - lets the widget pick the Wall, Floor, or Ceiling child blueprint
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TSubclassOf<ABuildingPart> BuildPartClass;

	// The building part that is currently spawned and being placed
	UPROPERTY()
	ABuildingPart* spawnedPart;

	// Which part type spawnedPart is - same indices as BuildingArray (0 Wall, 1 Floor,
	// 2 Ceiling) - so placement in Tick can treat floors and ceilings differently
	UPROPERTY()
	int spawnedPartID;

	// The live player HUD widget - set to a real instance on BeginPlay so the
	// Tick update always has a widget to write into
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UPlayerWidget* playerUI;

	// The Player_W widget blueprint the HUD instance is created from
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UPlayerWidget> PlayerHUDClass;

	// The live objective tracker widget - set to a real instance on BeginPlay,
	// then fed the collected and built totals as they change
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	UObjectiveWidget* objWidget;

	// The Objective_W widget blueprint the tracker instance is created from
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Widgets")
	TSubclassOf<UObjectiveWidget> ObjectiveHUDClass;

	// Running total of materials collected toward the 500 mats objective
	UPROPERTY()
	float matsCollected;

	// Running total of building parts placed toward the 5 parts objective
	UPROPERTY()
	float objectsBuilt;

	// Parts lifted by the floor preview while the plate slides underneath them -
	// raised one plate height right away and set back down if the preview leaves
	UPROPERTY()
	TArray<AActor*> previewLifted;

	// How far the current preview lift raised those parts
	float previewLiftHeight = 0.0f;

	// Keeps the lifted set in sync with whatever stands on the preview plate
	UFUNCTION()
	void UpdatePreviewLift();

	// Highest mesh top among placed parts overlapping a plate footprint at the
	// given spot - how a ceiling finds the walls it should climb onto
	float TallestSupportTop(FVector AtLocation, FVector PlateExtent, float MinTop);

	// Spends wood and stone to craft a building part - the buildingObject string
	// decides which slot of the building array goes up by one
	UFUNCTION(BlueprintCallable)
	void UpdateResources(float woodAmount, float stoneAmount, FString buildingObject);

	// Spawns the selected building part 400 units ahead if one is available -
	// isSuccess reports back to the widget whether the build happened
	UFUNCTION(BlueprintCallable)
	void SpawnBuilding(int buildingID, bool& isSuccess);

	// Rotates the part being placed by 90 degrees - bound to the RotPart input
	UFUNCTION()
	void RotateBuilding();

	// The crafting menu widget class to open - set to the CraftingWidget blueprint on PlayerChar_BP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Crafting")
	TSubclassOf<class UUserWidget> CraftWidgetClass;

	// The live crafting menu instance while it is open
	UPROPERTY()
	class UUserWidget* CraftWidget;

	// Opens/closes the crafting menu - bound to the CraftMenu input (F), works like
	// the flip flop in the videos: first press opens, next press closes
	UFUNCTION()
	void ToggleCraftMenu();
};
