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

private:
	// Once both objectives are complete, wait two seconds and quit the game,
	// matching the AND -> Delay 2.0 -> Quit Game chain from the video
	void CheckObjectives();

	// Fired by the quit timer
	void QuitGame();

	// Timer handle driving the two second delay before quitting
	FTimerHandle QuitTimerHandle;
};
