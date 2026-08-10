#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
// The headline text block and the two buttons in the layout
#include "Components/TextBlock.h"
#include "Components/Button.h"
#include "WinLoseWidget.generated.h"

UCLASS()
class GAM312_YUN_2026_API UWinLoseWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// The big center line - set to Winner or LOSERRR!! when the screen opens
	UPROPERTY(meta = (BindWidget))
	UTextBlock* WinLoseText;

	// Retry reopens the map so the run starts over
	UPROPERTY(meta = (BindWidget))
	UButton* RetryButton;

	// Quit closes the game entirely
	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;

	// Writes the headline - the caller decides what the screen should say
	UFUNCTION(BlueprintCallable)
	void SetWinLoseText(const FString& message);

	// Binds the buttons and hands the mouse back to the player, like the
	// Construct event in the video - cursor on, input set to game and UI
	virtual void NativeConstruct() override;

	UFUNCTION()
	void OnRetryClicked();

	UFUNCTION()
	void OnQuitClicked();
};
