#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
// Progress bar widgets the HUD updates every frame
#include "Components/ProgressBar.h"
#include "PlayerWidget.generated.h"

UCLASS()
class GAM312_YUN_2026_API UPlayerWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// The three stat bars laid out in the Player_W designer - BindWidget pairs
	// each pointer with the progress bar of the same name
	UPROPERTY(meta = (BindWidget))
	class UProgressBar* healthBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* hungerBar;

	UPROPERTY(meta = (BindWidget))
	class UProgressBar* staminaBar;

	// Called by the player character to refresh the bars - progress bars work
	// between zero and one, so each stat out of 100 gets divided down
	UFUNCTION(BlueprintCallable)
	void UpdateBars(float health, float hunger, float stamina);

	// The win and lose screen blueprint to open - set to WinLose_W on Player_W.
	// UpdateBars runs constantly, so it doubles as the health-zero watcher
	// from the video: health at or below zero pops the lose screen
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Win Lose")
	TSubclassOf<class UUserWidget> WinLoseClass;

	// Guards against opening the lose screen more than once
	bool bLoseShown = false;
};
