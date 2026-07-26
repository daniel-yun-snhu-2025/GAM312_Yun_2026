#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "WanderAIController.generated.h"

UCLASS()
class GAM312_YUN_2026_API AWanderAIController : public AAIController
{
	GENERATED_BODY()

public:
	// Kick off the first wander as soon as play begins, like calling the
	// Wander event off Event BeginPlay in the video
	virtual void BeginPlay() override;

	// Pick a random reachable point around the pawn and AI-move to it
	UFUNCTION()
	void Wander();

	// Fires when an AI Move To finishes - on success the video waits a random
	// one to three seconds and then wanders again
	virtual void OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result) override;

	// How far from the current spot each random destination may be
	UPROPERTY(EditAnywhere, Category = "Wander")
	float WanderRadius = 1500.0f;

	// Timer handle driving the pause between wanders
	FTimerHandle WanderTimerHandle;
};
