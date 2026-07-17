#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
// Widget component types used by the crafting menu
#include "Components/Button.h"
#include "Components/TextBlock.h"
// Building part class used by the build class properties
#include "BuildingPart.h"
#include "CraftingWidgetBase.generated.h"

class APlayerChar;

/**
 * Base class for the crafting widget. The designer layout lives in the
 * CraftingWidget blueprint - the BindWidget properties below connect to the
 * buttons and text blocks in that layout by name, and this class drives all
 * of the crafting menu logic: crafting parts, building them, eating berries,
 * and keeping the supply/current text up to date.
 */
UCLASS()
class GAM312_YUN_2026_API UCraftingWidgetBase : public UUserWidget
{
	GENERATED_BODY()

public:
	// Called when the widget is constructed - binds the buttons and grabs the player
	virtual void NativeConstruct() override;

	// ── Craftables column buttons (craft one part if you can afford it) ──
	UPROPERTY(meta = (BindWidget))
	UButton* wallButton;

	UPROPERTY(meta = (BindWidget))
	UButton* floorButton;

	UPROPERTY(meta = (BindWidget))
	UButton* ceilingButton;

	// ── Building menu buttons (spawn a crafted part for placement) ──
	UPROPERTY(meta = (BindWidget))
	UButton* buildwallButton;

	UPROPERTY(meta = (BindWidget))
	UButton* buildfloorButton;

	UPROPERTY(meta = (BindWidget))
	UButton* buildceilingButton;

	// ── Berry box ──
	UPROPERTY(meta = (BindWidget))
	UButton* berriesButton;

	// ── Text blocks the logic updates ──
	UPROPERTY(meta = (BindWidget))
	UTextBlock* currentWood;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* currentStone;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* berriesTxt;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* currentSupplies;

	// Reference to the player character so we can read arrays and call its events
	UPROPERTY(BlueprintReadWrite)
	APlayerChar* PlayerCharRef;

	// The child blueprint each build button spawns - set on the widget blueprint's
	// defaults to the Wall_BP, Floor_BP, and Ceiling_BP children of BuildingPart_BP
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Classes")
	TSubclassOf<ABuildingPart> WallClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Classes")
	TSubclassOf<ABuildingPart> FloorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Build Classes")
	TSubclassOf<ABuildingPart> CeilingClass;

	// Refreshes the building supply text (Wall/Floor/Ceiling counts)
	UFUNCTION(BlueprintCallable)
	void UpdateSupplies();

	// Refreshes the current resource text (Wood/Stone/Berries counts)
	UFUNCTION(BlueprintCallable)
	void UpdateCurrent();

	// ── Craft button handlers - spend resources, gain a building part ──
	UFUNCTION()
	void OnWallClicked();

	UFUNCTION()
	void OnFloorClicked();

	UFUNCTION()
	void OnCeilingClicked();

	// ── Build button handlers - spawn the part and close the menu ──
	UFUNCTION()
	void OnBuildWallClicked();

	UFUNCTION()
	void OnBuildFloorClicked();

	UFUNCTION()
	void OnBuildCeilingClicked();

	// ── Berry button handler - eat a berry to restore stats ──
	UFUNCTION()
	void OnBerriesClicked();

private:
	// Shared logic for the three craft buttons - the Remove Resources macro from the videos
	void RemoveResources(float woodAmount, float stoneAmount, FString buildingObject);

	// Shared logic for the three build buttons - spawns the part and closes the menu on success
	void BuildPart(int buildingID, TSubclassOf<ABuildingPart> partClass);
};
