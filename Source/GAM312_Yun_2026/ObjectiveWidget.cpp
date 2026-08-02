#include "ObjectiveWidget.h"
// QuitGame helper
#include "Kismet/KismetSystemLibrary.h"
// GetPlayerController for the quit call
#include "Kismet/GameplayStatics.h"

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
	// Both objectives complete - wait two seconds, then quit the game
	if (MatComplete && BuildComplete && !QuitTimerHandle.IsValid())
	{
		GetWorld()->GetTimerManager().SetTimer(QuitTimerHandle, this, &UObjectiveWidget::QuitGame, 2.0f, false);
	}
}

void UObjectiveWidget::QuitGame()
{
	UKismetSystemLibrary::QuitGame(GetWorld(), UGameplayStatics::GetPlayerController(GetWorld(), 0), EQuitPreference::Quit, false);
}
