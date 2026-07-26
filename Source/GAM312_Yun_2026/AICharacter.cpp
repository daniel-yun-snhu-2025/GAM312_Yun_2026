#include "AICharacter.h"
// The wander controller this character is assigned in Class Defaults
#include "WanderAIController.h"

AAICharacter::AAICharacter()
{
	// Nothing per-frame on the character itself - the controller does the work
	PrimaryActorTick.bCanEverTick = false;

	// Set the AI Controller to our wander controller, and possess whether the
	// AI is placed in the world or spawned - same as the Class Defaults setup
	AIControllerClass = AWanderAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}
