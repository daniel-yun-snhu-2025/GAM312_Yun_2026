#include "CraftingWidgetBase.h"
// Player character - the widget reads its arrays and calls its crafting events
#include "PlayerChar.h"
// GetPlayerCharacter
#include "Kismet/GameplayStatics.h"
// Input mode switching when the menu closes
#include "GameFramework/PlayerController.h"

void UCraftingWidgetBase::NativeConstruct()
{
	Super::NativeConstruct();

	// Cast the player character to our class and keep the reference,
	// just like the Cast To PlayerChar_BP promoted to a variable in the videos
	PlayerCharRef = Cast<APlayerChar>(UGameplayStatics::GetPlayerCharacter(GetWorld(), 0));

	// Bind every button's OnClicked to its handler
	if (wallButton) { wallButton->OnClicked.AddUniqueDynamic(this, &UCraftingWidgetBase::OnWallClicked); }
	if (floorButton) { floorButton->OnClicked.AddUniqueDynamic(this, &UCraftingWidgetBase::OnFloorClicked); }
	if (ceilingButton) { ceilingButton->OnClicked.AddUniqueDynamic(this, &UCraftingWidgetBase::OnCeilingClicked); }
	if (buildwallButton) { buildwallButton->OnClicked.AddUniqueDynamic(this, &UCraftingWidgetBase::OnBuildWallClicked); }
	if (buildfloorButton) { buildfloorButton->OnClicked.AddUniqueDynamic(this, &UCraftingWidgetBase::OnBuildFloorClicked); }
	if (buildceilingButton) { buildceilingButton->OnClicked.AddUniqueDynamic(this, &UCraftingWidgetBase::OnBuildCeilingClicked); }
	if (berriesButton) { berriesButton->OnClicked.AddUniqueDynamic(this, &UCraftingWidgetBase::OnBerriesClicked); }

	// Populate the text as soon as the menu opens, like Update Current /
	// Update Supplies off Event Construct in the videos
	UpdateCurrent();
	UpdateSupplies();
}

void UCraftingWidgetBase::UpdateSupplies()
{
	// Building supply readout - Wall, Floor, Ceiling counts from the building array
	if (PlayerCharRef && currentSupplies)
	{
		FString Supplies = FString::Printf(TEXT("Wall: %d\n\nFloor: %d\n\nCeiling: %d"),
			PlayerCharRef->BuildingArray[0], PlayerCharRef->BuildingArray[1], PlayerCharRef->BuildingArray[2]);
		currentSupplies->SetText(FText::FromString(Supplies));
	}
}

void UCraftingWidgetBase::UpdateCurrent()
{
	// Current resource readout - Wood, Stone, and Berries from the resources array
	if (PlayerCharRef)
	{
		if (currentWood)
		{
			currentWood->SetText(FText::FromString(FString::Printf(TEXT("Wood: %d"), PlayerCharRef->ResourcesArray[0])));
		}
		if (currentStone)
		{
			currentStone->SetText(FText::FromString(FString::Printf(TEXT("Stone: %d"), PlayerCharRef->ResourcesArray[1])));
		}
		if (berriesTxt)
		{
			berriesTxt->SetText(FText::FromString(FString::Printf(TEXT("Berries: %d"), PlayerCharRef->ResourcesArray[2])));
		}
	}
}

// The Remove Resources macro from the videos - spend the cost, then refresh both readouts
void UCraftingWidgetBase::RemoveResources(float woodAmount, float stoneAmount, FString buildingObject)
{
	if (PlayerCharRef)
	{
		PlayerCharRef->UpdateResources(woodAmount, stoneAmount, buildingObject);
		UpdateCurrent();
		UpdateSupplies();
	}
}

// Craft costs match the Cost column: Wall 10/5, Floor 30/15, Ceiling 50/30
void UCraftingWidgetBase::OnWallClicked()
{
	RemoveResources(10.0f, 5.0f, "Wall");
}

void UCraftingWidgetBase::OnFloorClicked()
{
	RemoveResources(30.0f, 15.0f, "Floor");
}

void UCraftingWidgetBase::OnCeilingClicked()
{
	RemoveResources(50.0f, 30.0f, "Ceiling");
}

// Build buttons - set the part class on the player, spawn it, and close the menu on success
void UCraftingWidgetBase::BuildPart(int buildingID, TSubclassOf<ABuildingPart> partClass)
{
	if (!PlayerCharRef)
	{
		return;
	}

	// Which child blueprint SpawnBuilding should spawn (Wall, Floor, or Ceiling)
	PlayerCharRef->BuildPartClass = partClass;

	bool isSuccess = false;
	PlayerCharRef->SpawnBuilding(buildingID, isSuccess);

	if (isSuccess)
	{
		UpdateSupplies();

		// Hand control back to the game so the part can be placed without clicking through UI -
		// hide the cursor, set input to game only, and remove this widget from the viewport
		if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
		{
			PC->bShowMouseCursor = false;
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
		}

		RemoveFromParent();
	}
}

void UCraftingWidgetBase::OnBuildWallClicked()
{
	BuildPart(0, WallClass);
}

void UCraftingWidgetBase::OnBuildFloorClicked()
{
	BuildPart(1, FloorClass);
}

void UCraftingWidgetBase::OnBuildCeilingClicked()
{
	BuildPart(2, CeilingClass);
}

void UCraftingWidgetBase::OnBerriesClicked()
{
	// Eating a berry - only if we actually have one
	if (PlayerCharRef && PlayerCharRef->ResourcesArray[2] > 0)
	{
		// Take one berry out of the inventory
		PlayerCharRef->ResourcesArray[2] = PlayerCharRef->ResourcesArray[2] - 1;

		// A berry restores 10 health, 50 hunger, and 50 stamina
		PlayerCharRef->SetHealth(10.0f);
		PlayerCharRef->SetHunger(50.0f);
		PlayerCharRef->SetStamina(50.0f);

		UpdateCurrent();
	}
}
