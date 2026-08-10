#include "WinLoseWidget.h"
// Open Level for retry and Get Player Controller for the input mode
#include "Kismet/GameplayStatics.h"
// Quit Game helper
#include "Kismet/KismetSystemLibrary.h"
#include "GameFramework/PlayerController.h"

void UWinLoseWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Wire both buttons to their handlers
	if (RetryButton)
	{
		RetryButton->OnClicked.AddUniqueDynamic(this, &UWinLoseWidget::OnRetryClicked);
	}

	if (QuitButton)
	{
		QuitButton->OnClicked.AddUniqueDynamic(this, &UWinLoseWidget::OnQuitClicked);
	}

	// Show the cursor and switch input to game and UI so the buttons are usable
	if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
	{
		PC->bShowMouseCursor = true;
		FInputModeGameAndUI InputMode;
		InputMode.SetWidgetToFocus(TakeWidget());
		PC->SetInputMode(InputMode);
	}
}

void UWinLoseWidget::SetWinLoseText(const FString& message)
{
	if (WinLoseText)
	{
		WinLoseText->SetText(FText::FromString(message));
	}
}

void UWinLoseWidget::OnRetryClicked()
{
	// Reopen the map by name to restart the whole run
	UGameplayStatics::OpenLevel(GetWorld(), FName(TEXT("testMap")));
}

void UWinLoseWidget::OnQuitClicked()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(), 0), EQuitPreference::Quit, false);
}
