#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
// Text blocks the objective lines are written into
#include "Components/TextBlock.h"
#include "ObjectiveWidget.generated.h"

UCLASS()
class GAM312_YUN_2026_API UObjectiveWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// The two objective lines from the Objective_W layout - BindWidget pairs
	// each pointer with the text block of the same name in the designer
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* matObjectiveTxt;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* buildObjectiveTxt;

	// True once each objective has been met - both true quits the game
	UPROPERTY(BlueprintReadOnly, Category = "Objectives")
	bool MatComplete = false;

	UPROPERTY(BlueprintReadOnly, Category = "Objectives")
	bool BuildComplete = false;

	// Refreshes the material objective line - shows the running count and
	// flips to Complete once 500 mats have been collected
	UFUNCTION(BlueprintCallable)
	void UpdatematOBJ(float matsCollected);

	// Refreshes the build objective line - complete once 5 parts are placed
	UFUNCTION(BlueprintCallable)
	void UpdatebuildObj(float objectsBuilt);

	// The win and lose screen blueprint to open - set to WinLose_W on Objective_W
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Win Lose")
	TSubclassOf<class UUserWidget> WinLoseClass;

private:
	// Once both objectives are complete, pop the win screen - this replaces
	// the straight Quit Game the objectives used before this module
	void CheckObjectives();

	// Creates the win lose widget, sets its headline to Winner, and shows it
	void ShowWinScreen();

	// Guards against opening the win screen more than once
	bool bWinShown = false;
};
