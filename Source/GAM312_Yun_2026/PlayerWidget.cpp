#include "PlayerWidget.h"

// Push the current stat values into the three bars - health 90 out of 100
// becomes 0.9 on the bar, and the same for hunger and stamina
void UPlayerWidget::UpdateBars(float health, float hunger, float stamina)
{
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
