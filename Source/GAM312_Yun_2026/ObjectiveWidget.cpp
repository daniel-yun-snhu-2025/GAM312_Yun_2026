#include "ObjectiveWidget.h"
// The win and lose screen this widget opens on completion
#include "WinLoseWidget.h"

void UObjectiveWidget::UpdatematOBJ(float matsCollected)
{
	if (!matObjectiveTxt)
	{
		return;
	}

	if (matsCollected >= 500.0f)
	{
		// Objective met - the >= 500 branch sets the line to Complete
		matObjectiveTxt->SetText(FText::FromString(TEXT("Complete!")));
		MatComplete = true;
		CheckObjectives();
	}
	else
	{
		// Append the running count to the objective line
		FString Line = FString::Printf(TEXT("Collect 500 Mats: %d"), FMath::RoundToInt(matsCollected));
		matObjectiveTxt->SetText(FText::FromString(Line));
	}
}

void UObjectiveWidget::UpdatebuildObj(float objectsBuilt)
{
	if (!buildObjectiveTxt)
	{
		return;
	}

	if (objectsBuilt >= 5.0f)
	{
		// Objective met - five parts placed flips the line to Complete
		buildObjectiveTxt->SetText(FText::FromString(TEXT("Complete!")));
		BuildComplete = true;
		CheckObjectives();
	}
	else
	{
		FString Line = FString::Printf(TEXT("Build 5 Parts: %d"), FMath::RoundToInt(objectsBuilt));
		buildObjectiveTxt->SetText(FText::FromString(Line));
	}
}

void UObjectiveWidget::CheckObjectives()
{
	// Both objectives complete - open the win screen once
	if (MatComplete && BuildComplete && !bWinShown)
	{
		bWinShown = true;
		ShowWinScreen();
	}
}

void UObjectiveWidget::ShowWinScreen()
{
	// Create the win lose widget, tell it we won, and put it on screen -
	// same Create Widget, Set Win Lose Text, Add to Viewport chain as the video
	if (WinLoseClass)
	{
		if (UWinLoseWidget* WinScreen = CreateWidget<UWinLoseWidget>(GetWorld(), WinLoseClass))
		{
			WinScreen->SetWinLoseText(TEXT("Winner"));
			WinScreen->AddToViewport();
		}
	}
}
