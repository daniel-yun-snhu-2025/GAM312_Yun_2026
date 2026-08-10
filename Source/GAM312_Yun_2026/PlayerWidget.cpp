#include "PlayerWidget.h"
// The lose screen this widget opens when health runs out
#include "WinLoseWidget.h"

// Push the current stat values into the three bars - health 90 out of 100
// becomes 0.9 on the bar, and the same for hunger and stamina
void UPlayerWidget::UpdateBars(float health, float hunger, float stamina)
{
	// Health hit zero - the run is lost, so open the win lose screen once
	// with the lose message, the same chain the objective widget uses to win
	if (health <= 0.0f && !bLoseShown && WinLoseClass)
	{
		bLoseShown = true;

		if (UWinLoseWidget* LoseScreen = CreateWidget<UWinLoseWidget>(GetWorld(), WinLoseClass))
		{
			LoseScreen->SetWinLoseText(TEXT("LOSERRR!!"));
			LoseScreen->AddToViewport();
		}
	}

	if (healthBar)
	{
		healthBar->SetPercent(health / 100.0f);
	}

	if (hungerBar)
	{
		hungerBar->SetPercent(hunger / 100.0f);
	}

	if (staminaBar)
	{
		staminaBar->SetPercent(stamina / 100.0f);
	}
}
