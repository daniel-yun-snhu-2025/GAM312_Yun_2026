#include "WanderAIController.h"
// Navigation system supplies the random reachable points on the nav mesh
#include "NavigationSystem.h"

void AWanderAIController::BeginPlay()
{
	Super::BeginPlay();

	// Start wandering right away
	Wander();
}

void AWanderAIController::Wander()
{
	APawn* ControlledPawn = GetPawn();
	if (!ControlledPawn)
	{
		return;
	}

	// Same as Get Random Location In Navigable Radius in the video - the
	// origin is the AI's own location, and the random point is the destination.
	// This is where the navigation mesh does the work: the NavMeshBoundsVolume
	// bakes the level's walkable surfaces into connected polygons, and
	// GetRandomReachablePointInRadius samples a point guaranteed to lie on
	// polygons reachable from the pawn - so the AI never picks a spot inside
	// a rock or off the terrain
	FNavLocation RandomLocation;
	UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());

	if (NavSystem && NavSystem->GetRandomReachablePointInRadius(ControlledPawn->GetActorLocation(), WanderRadius, RandomLocation))
	{
		// MoveToLocation is the pathfinding call: it runs A* across the nav
		// mesh's polygon graph to build a path, then the controller steers the
		// character along it - obstacles and slopes are already encoded in the
		// baked mesh, so following the path avoids them by construction
		MoveToLocation(RandomLocation.Location);
	}
	else
	{
		// No usable nav data yet - try again shortly instead of going idle
		GetWorldTimerManager().SetTimer(WanderTimerHandle, this, &AWanderAIController::Wander, 1.0f, false);
	}
}

void AWanderAIController::OnMoveCompleted(FAIRequestID RequestID, const FPathFollowingResult& Result)
{
	Super::OnMoveCompleted(RequestID, Result);

	// Delay for a random float in range one to three, then call Wander again
	GetWorldTimerManager().SetTimer(WanderTimerHandle, this, &AWanderAIController::Wander, FMath::RandRange(1.0f, 3.0f), false);
}
