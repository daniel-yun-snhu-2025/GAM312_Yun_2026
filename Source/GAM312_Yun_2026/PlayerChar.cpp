#include "PlayerChar.h"
#include "Camera/CameraComponent.h"
// Crafting menu widget creation and input mode switching
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"

// Sets default values
APlayerChar::APlayerChar()
{
	// Set this character to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Camera libraries in a game engine hide the projection math behind a
	// component: UCameraComponent carries the field of view and world transform,
	// and every frame the engine builds the view and projection matrices from it
	// to turn 3D world positions into what appears on screen. Creating it as a
	// default subobject makes the camera part of this character's composition.
	PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));

	// Attaching the camera to the head bone puts the lens where the character's
	// eyes are - the definition of a first-person perspective. The component
	// inherits the socket's transform, so the view rides the character for free
	PlayerCamComp->SetupAttachment(GetMesh(), "head");

	// Using the controller's rotation for the camera - mouse input accumulates in
	// the controller, the pawn shares that rotation, and the camera library reads
	// it back each frame, so looking around is just rotating one transform that
	// the view matrix is rebuilt from (true first person)
	PlayerCamComp->bUsePawnControlRotation = true;

	// Establishing the resource arrays - three elements matching Wood, Stone, and Berry
	ResourcesArray.SetNum(3);
	ResourcesNameArray.Add(TEXT("Wood"));
	ResourcesNameArray.Add(TEXT("Stone"));
	ResourcesNameArray.Add(TEXT("Berry"));

	// Establishing the building array - three elements matching Wall, Floor, and Ceiling
	BuildingArray.SetNum(3);
}

// Called when the game starts or when spawned
void APlayerChar::BeginPlay()
{
	Super::BeginPlay();

	// Starting the stats timer - calls DecreaseStats every 2 seconds, looping
	GetWorld()->GetTimerManager().SetTimer(StatsTimerHandle, this, &APlayerChar::DecreaseStats, 2.0f, true);

	// Create the player HUD from the Player_W class, keep the instance in
	// playerUI, and add it to the viewport - the reference must point at a
	// live instance before Tick starts pushing stats into it
	if (PlayerHUDClass)
	{
		playerUI = CreateWidget<UPlayerWidget>(GetWorld(), PlayerHUDClass);

		if (playerUI)
		{
			playerUI->AddToViewport();
		}
	}

	// Create the objective tracker the same way and seed both objective
	// lines at zero until things are added to them
	if (ObjectiveHUDClass)
	{
		objWidget = CreateWidget<UObjectiveWidget>(GetWorld(), ObjectiveHUDClass);

		if (objWidget)
		{
			objWidget->AddToViewport();

			objWidget->UpdatebuildObj(0.0f);
			objWidget->UpdatematOBJ(0.0f);
		}
	}
}

// Called every frame
void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Push the current stats into the HUD bars every frame, like the video's
	// Update Bars call off the Tick event
	if (playerUI)
	{
		playerUI->UpdateBars(Health, Hunger, Stamina);
	}

	// While placing a building part, keep it ahead of the camera so it follows
	// wherever the player points until they click to place it.
	// This whole block is applied linear algebra: every position and direction
	// is a 3D vector, "ahead of the camera" is the camera position plus its
	// forward unit vector scaled by a distance (vector addition and scalar
	// multiplication), the snap logic below compares vector components against
	// box extents, and every lift or drop is a vector sum along the world Z axis
	if (isBuilding)
	{
		if (spawnedPart)
		{
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;
			FVector EndLocation = StartLocation + Direction;

			FCollisionQueryParams TraceParams;
			TraceParams.AddIgnoredActor(this);
			TraceParams.AddIgnoredActor(spawnedPart);

			// The mesh can sit away from the actor pivot (the plates hang off their
			// pivot edge), so all of the snapping below works out where the MESH
			// belongs and then shifts the pivot by the current mesh offset
			FVector MeshOffset = spawnedPart->Mesh->Bounds.Origin - spawnedPart->GetActorLocation();
			FVector SpawnedExtent = spawnedPart->Mesh->Bounds.BoxExtent;

			// First trace along the camera's aim, out to the same 800 unit reach as
			// the harvest trace, to find what the player is looking at
			FHitResult AimHit;
			if (GetWorld()->LineTraceSingleByChannel(AimHit, StartLocation, StartLocation + PlayerCamComp->GetForwardVector() * 800.0f, ECC_Visibility, TraceParams))
			{
				EndLocation = AimHit.Location;

				// Looking at another building part - where the new part snaps depends
				// on what is being placed: floors slide in underneath the part, while
				// walls and ceilings stack on top of it
				if (ABuildingPart* HitPart = Cast<ABuildingPart>(AimHit.GetActor()))
				{
					// Bounds come straight from the mesh - actor bounds would come back
					// empty for a part whose collision is off, like the preview itself
					FVector PartOrigin = HitPart->Mesh->Bounds.Origin;
					FVector PartExtent = HitPart->Mesh->Bounds.BoxExtent;

					// Measure from the part's resting base - if the floor preview has
					// already lifted it, ignore that lift so the plate stays anchored
					float PartBottom = PartOrigin.Z - PartExtent.Z;
					if (previewLifted.Contains(HitPart))
					{
						PartBottom = PartBottom - previewLiftHeight;
					}

					// Plates (floor and ceiling) line up flush against a wall - the wall
					// stands along the plate's edge with matching corners, and the plate
					// runs out toward the player's side. Everything else locks onto the
					// part's center so the preview cannot jitter with the aim point.
					bool bPlatePart = (spawnedPartID == 1 || spawnedPartID == 2);
					bool bWallTarget = PartExtent.Z >= 100.0f;

					if (bPlatePart && bWallTarget)
					{
						// The wall's thin horizontal axis decides which way the plate lies
						bool bThinX = PartExtent.X <= PartExtent.Y;

						if (bThinX)
						{
							float PlayerSide = GetActorLocation().X >= PartOrigin.X ? 1.0f : -1.0f;
							EndLocation.X = PartOrigin.X + PlayerSide * (SpawnedExtent.X - PartExtent.X);
							EndLocation.Y = PartOrigin.Y;
						}
						else
						{
							float PlayerSide = GetActorLocation().Y >= PartOrigin.Y ? 1.0f : -1.0f;
							EndLocation.X = PartOrigin.X;
							EndLocation.Y = PartOrigin.Y + PlayerSide * (SpawnedExtent.Y - PartExtent.Y);
						}
					}
					else
					{
						EndLocation.X = PartOrigin.X;
						EndLocation.Y = PartOrigin.Y;
					}

					if (spawnedPartID == 1)
					{
						// Floor - the plate slides in at the part's resting base
						EndLocation.Z = PartBottom + SpawnedExtent.Z;
					}
					else
					{
						// Wall or ceiling - the new part itself settles with its own
						// underside on the support's top
						float SupportTop = PartOrigin.Z + PartExtent.Z;

						// A ceiling seats on the tallest part standing inside its
						// footprint - so aiming at the floor of a room still lands
						// the roof up on the walls, not down on the floor plate
						if (spawnedPartID == 2)
						{
							SupportTop = TallestSupportTop(EndLocation, SpawnedExtent, SupportTop);
						}

						EndLocation.Z = SupportTop + SpawnedExtent.Z;
					}

					// EndLocation is where the mesh center belongs - shift the pivot
					spawnedPart->SetActorLocation(EndLocation - MeshOffset);

					// A floor preview lifts what stands on its plate the moment it
					// slides underneath - no click needed
					if (spawnedPartID == 1)
					{
						UpdatePreviewLift();
					}

					return;
				}
			}

			// Otherwise drop straight down from just above the aim point so the
			// mesh rests level on the supporting surface
			FHitResult GroundHit;
			if (GetWorld()->LineTraceSingleByChannel(GroundHit, EndLocation + FVector(0.0f, 0.0f, 50.0f), EndLocation - FVector(0.0f, 0.0f, 2000.0f), ECC_Visibility, TraceParams))
			{
				// Seat the mesh's underside on the ground
				EndLocation.Z = GroundHit.Location.Z + SpawnedExtent.Z;
			}

			// A ceiling climbs onto the tallest part standing in its footprint on
			// its own - the player never has to aim at the wall to roof a shelter
			if (spawnedPartID == 2)
			{
				float SupportTop = TallestSupportTop(EndLocation, SpawnedExtent, EndLocation.Z - SpawnedExtent.Z);
				EndLocation.Z = SupportTop + SpawnedExtent.Z;
			}

			// EndLocation is where the mesh center belongs - shift the pivot
			spawnedPart->SetActorLocation(EndLocation - MeshOffset);

			// Keep the floor preview's lift in sync here too, so parts settle
			// back down as soon as the plate slides away from them
			if (spawnedPartID == 1)
			{
				UpdatePreviewLift();
			}
		}
	}
}

float APlayerChar::TallestSupportTop(FVector AtLocation, FVector PlateExtent, float MinTop)
{
	TArray<AActor*> Parts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildingPart::StaticClass(), Parts);

	for (AActor* Part : Parts)
	{
		ABuildingPart* OtherPart = Cast<ABuildingPart>(Part);
		if (!OtherPart || Part == spawnedPart)
		{
			continue;
		}

		FVector OtherOrigin = OtherPart->Mesh->Bounds.Origin;
		FVector OtherExtent = OtherPart->Mesh->Bounds.BoxExtent;

		// Overlapping the plate's footprint in X and Y
		bool bInFootprint = FMath::Abs(OtherOrigin.X - AtLocation.X) <= (OtherExtent.X + PlateExtent.X)
			&& FMath::Abs(OtherOrigin.Y - AtLocation.Y) <= (OtherExtent.Y + PlateExtent.Y);

		if (bInFootprint && (OtherOrigin.Z + OtherExtent.Z) > MinTop)
		{
			MinTop = OtherOrigin.Z + OtherExtent.Z;
		}
	}

	return MinTop;
}

void APlayerChar::UpdatePreviewLift()
{
	if (!spawnedPart)
	{
		return;
	}

	// Mesh bounds again - the preview's collision is off, so actor bounds are empty
	FVector FloorOrigin = spawnedPart->Mesh->Bounds.Origin;
	FVector FloorExtent = spawnedPart->Mesh->Bounds.BoxExtent;
	float LiftHeight = FloorExtent.Z * 2.0f;

	TArray<AActor*> Parts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABuildingPart::StaticClass(), Parts);

	// Everything currently standing on the preview plate's footprint
	TArray<AActor*> NowOnPlate;
	for (AActor* Part : Parts)
	{
		ABuildingPart* OtherPart = Cast<ABuildingPart>(Part);
		if (!OtherPart || Part == spawnedPart)
		{
			continue;
		}

		FVector PartOrigin = OtherPart->Mesh->Bounds.Origin;
		FVector PartExtent = OtherPart->Mesh->Bounds.BoxExtent;

		bool bOverlapsXY = FMath::Abs(PartOrigin.X - FloorOrigin.X) <= (PartExtent.X + FloorExtent.X)
			&& FMath::Abs(PartOrigin.Y - FloorOrigin.Y) <= (PartExtent.Y + FloorExtent.Y);
		bool bOnOrAboveBase = (PartOrigin.Z - PartExtent.Z) >= (FloorOrigin.Z - FloorExtent.Z) - 1.0f;

		if (bOverlapsXY && bOnOrAboveBase)
		{
			NowOnPlate.Add(Part);
		}
	}

	// Parts the plate slid away from settle back down
	for (int32 i = previewLifted.Num() - 1; i >= 0; i--)
	{
		AActor* Part = previewLifted[i];
		if (!Part || !NowOnPlate.Contains(Part))
		{
			if (Part)
			{
				Part->AddActorWorldOffset(FVector(0.0f, 0.0f, -previewLiftHeight));
			}
			previewLifted.RemoveAt(i);
		}
	}

	// Parts the plate just slid underneath rise onto it right away
	for (AActor* Part : NowOnPlate)
	{
		if (!previewLifted.Contains(Part))
		{
			Part->AddActorWorldOffset(FVector(0.0f, 0.0f, LiftHeight));
			previewLifted.Add(Part);
		}
	}

	previewLiftHeight = LiftHeight;
}

// Called to bind functionality to input
void APlayerChar::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Setting up the axis inputs for movement
	PlayerInputComponent->BindAxis("MoveForward", this, &APlayerChar::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &APlayerChar::MoveRight);

	// LookUp and Turn rotate the controller itself (Controller Pitch/Yaw Input),
	// and the pawn simply shares that rotation - no custom functions needed
	PlayerInputComponent->BindAxis("LookUp", this, &APlayerChar::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("Turn", this, &APlayerChar::AddControllerYawInput);

	// Setting up the action inputs
	// JumpEvent calls our custom StartJump/StopJump functions on press and release
	PlayerInputComponent->BindAction("JumpEvent", IE_Pressed, this, &APlayerChar::StartJump);
	PlayerInputComponent->BindAction("JumpEvent", IE_Released, this, &APlayerChar::StopJump);

	// Interact calls the FindObject function - this is how the input for the FindObject event is set up
	PlayerInputComponent->BindAction("Interact", IE_Pressed, this, &APlayerChar::FindObject);

	// RotPart rotates the building part currently being placed
	PlayerInputComponent->BindAction("RotPart", IE_Pressed, this, &APlayerChar::RotateBuilding);

	// CraftMenu opens and closes the crafting menu
	PlayerInputComponent->BindAction("CraftMenu", IE_Pressed, this, &APlayerChar::ToggleCraftMenu);
}

void APlayerChar::MoveForward(float axisValue)
{
	// Using RotationMatrix to get the control rotation of the character and establishing
	// the forward axis (X) they are going to be moving in based on that control rotation
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::X);

	// The axis value passed in (+1 or -1) moves the character forward or backward
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::MoveRight(float axisValue)
{
	// Same as MoveForward, except we are using the right axis (Y) for left and right movement
	FVector Direction = FRotationMatrix(Controller->GetControlRotation()).GetScaledAxis(EAxis::Y);
	AddMovementInput(Direction, axisValue);
}

void APlayerChar::StartJump()
{
	// Unreal has a jump function built into Character classes - setting this flag to true
	// while Space Bar is pressed triggers that built-in jump
	bPressedJump = true;
}

void APlayerChar::StopJump()
{
	// Clearing the jump flag when Space Bar is released
	bPressedJump = false;
}

void APlayerChar::FindObject()
{
	// While placing a building part, clicking should place it instead of tracing -
	// so the harvest trace only runs when we are not building
	if (isBuilding)
	{
		// Clicking while building places the part and stops the follow logic in Tick
		isBuilding = false;

		// The placed part becomes solid again now that placement is confirmed
		if (spawnedPart)
		{
			spawnedPart->SetActorEnableCollision(true);
		}

		// Parts the floor preview lifted stay resting on the placed plate
		previewLifted.Empty();

		// One more part placed toward the build objective
		objectsBuilt = objectsBuilt + 1.0f;

		if (objWidget)
		{
			objWidget->UpdatebuildObj(objectsBuilt);
		}

		return;
	}

	// Line trace setup - fires from the camera to whatever it is looking at.
	// A trace sweeps a ray through the physics scene and returns the first
	// primitive whose collision settings respond on the queried channel - here
	// ECC_Visibility. Each mesh's collision profile decides whether it blocks,
	// overlaps, or ignores that channel, which is how one ray can hit a resource
	// node, a placed wall, or the landscape, and pass through everything else
	FHitResult HitResult;
	FVector StartLocation = PlayerCamComp->GetComponentLocation();
	// 800 units ahead of the camera in the direction it is looking
	FVector Direction = PlayerCamComp->GetForwardVector() * 800.0f;
	FVector EndLocation = StartLocation + Direction;

	// The trace ignores the player character, sees complex collision, and can return face normals
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.bTraceComplex = true;
	QueryParams.bReturnFaceIndex = true;

	// Stamina requirement - the hit trace only runs while there is more than 5 stamina left
	if (Stamina > 5.0f)
	{
		// Performing the line trace on the Visibility channel
		if (GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, QueryParams))
		{
			// Cast the hit actor to our master resource class
			AResource_M* HitResource = Cast<AResource_M>(HitResult.GetActor());

			// Only continue if we actually hit a resource - hitting anything else would crash otherwise
			if (HitResource)
			{
				FString hitName = HitResource->resourceName;
				int resourceValue = HitResource->resourceAmount;

				// Subtract the amount this hit gives us from the resource's remaining total
				HitResource->totalResource = HitResource->totalResource - resourceValue;

				if (HitResource->totalResource > resourceValue)
				{
					// There is still resource left - give the player the value collected
					GiveResource(resourceValue, hitName);

					// Count the collected amount toward the material objective
					matsCollected = matsCollected + resourceValue;

					if (objWidget)
					{
						objWidget->UpdatematOBJ(matsCollected);
					}

					check(GEngine != nullptr);
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Green, TEXT("Resource Collected"));

					// Spawn the hit decal at the trace hit location as a visual indicator -
					// 10x10x10 size, -90 pitch so it faces up off the surface, 2 second lifespan
					UGameplayStatics::SpawnDecalAtLocation(GetWorld(), hitDecal, FVector(10.0f, 10.0f, 10.0f), HitResult.Location, FRotator(-90, 0, 0), 2.0f);

					// Collecting costs 5 stamina each hit
					SetStamina(-5.0f);
				}
				else
				{
					// Nothing left - destroy the resource actor
					HitResource->Destroy();
					check(GEngine != nullptr);
					GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Red, TEXT("Resource Depleted"));
				}
			}
		}
	}
}

void APlayerChar::GiveResource(float amount, FString resourceType)
{
	// The resource name decides which index of the array the amount is added to
	if (resourceType == "Wood")
	{
		ResourcesArray[0] = ResourcesArray[0] + amount;
	}

	if (resourceType == "Stone")
	{
		ResourcesArray[1] = ResourcesArray[1] + amount;
	}

	if (resourceType == "Berry")
	{
		ResourcesArray[2] = ResourcesArray[2] + amount;
	}
}

void APlayerChar::SetHealth(float amount)
{
	// Only apply the change while Health stays under 100 - keeps the stat from exceeding its cap
	if (Health + amount < 100)
	{
		Health = Health + amount;
	}
}

void APlayerChar::SetHunger(float amount)
{
	if (Hunger + amount < 100)
	{
		Hunger = Hunger + amount;
	}
}

void APlayerChar::SetStamina(float amount)
{
	if (Stamina + amount < 100)
	{
		Stamina = Stamina + amount;
	}
}

void APlayerChar::DecreaseStats()
{
	// Hunger goes down by 1 every interval while there is hunger left
	if (Hunger > 0)
	{
		SetHunger(-1.0f);
	}

	// Stamina regeneration - adds back 10 every interval
	SetStamina(10.0f);

	// Once Hunger is empty, Health drains fast - an ignored hunger bar
	// loses the run in well under a minute
	if (Hunger <= 0)
	{
		SetHealth(-5.0f);
	}
}

void APlayerChar::UpdateResources(float woodAmount, float stoneAmount, FString buildingObject)
{
	// Only craft if the player can afford the wood cost
	if (woodAmount <= ResourcesArray[0])
	{
		// And the stone cost
		if (stoneAmount <= ResourcesArray[1])
		{
			// Spend the resources
			ResourcesArray[0] = ResourcesArray[0] - woodAmount;
			ResourcesArray[1] = ResourcesArray[1] - stoneAmount;

			// The building object string decides which building slot goes up by one
			if (buildingObject == "Wall")
			{
				BuildingArray[0] = BuildingArray[0] + 1;
			}

			if (buildingObject == "Floor")
			{
				BuildingArray[1] = BuildingArray[1] + 1;
			}

			if (buildingObject == "Ceiling")
			{
				BuildingArray[2] = BuildingArray[2] + 1;
			}
		}
	}
}

void APlayerChar::SpawnBuilding(int buildingID, bool& isSuccess)
{
	// Can't start placing a new part while one is already being placed
	if (!isBuilding)
	{
		// Check that we actually crafted at least one of this part type
		if (BuildingArray[buildingID] >= 1)
		{
			isBuilding = true;
			FActorSpawnParameters SpawnParams;

			// Spawn the part 400 units ahead of the camera
			FVector StartLocation = PlayerCamComp->GetComponentLocation();
			FVector Direction = PlayerCamComp->GetForwardVector() * 400.0f;
			FVector EndLocation = StartLocation + Direction;
			FRotator myRot(0, 0, 0);

			// Start the part on the ground right away - trace down from the aim
			// point so the pivot lands level with the floor
			FHitResult GroundHit;
			FCollisionQueryParams GroundParams;
			GroundParams.AddIgnoredActor(this);

			if (GetWorld()->LineTraceSingleByChannel(GroundHit, EndLocation, EndLocation - FVector(0.0f, 0.0f, 2000.0f), ECC_Visibility, GroundParams))
			{
				EndLocation.Z = GroundHit.Location.Z;
			}

			// Spend one part from the building array since we are placing it
			BuildingArray[buildingID] = BuildingArray[buildingID] - 1;

			// Remember which part type this is so Tick knows how to snap it while placing
			spawnedPartID = buildingID;

			// BuildPartClass is set by the widget to the Wall, Floor, or Ceiling child blueprint
			spawnedPart = GetWorld()->SpawnActor<ABuildingPart>(BuildPartClass, EndLocation, myRot, SpawnParams);

			// The part is only a preview while it follows the view - collision stays
			// off until it is placed so it cannot shove the player around
			if (spawnedPart)
			{
				spawnedPart->SetActorEnableCollision(false);
			}

			isSuccess = true;
		}
		else
		{
			// No parts of this type available - report the failure back to the widget
			isSuccess = false;
		}
	}
}

void APlayerChar::RotateBuilding()
{
	// Rotate the part being placed 90 degrees at a time
	if (isBuilding)
	{
		spawnedPart->AddActorWorldRotation(FRotator(0, 90, 0));
	}
}

void APlayerChar::ToggleCraftMenu()
{
	APlayerController* PC = Cast<APlayerController>(GetController());

	// First press (menu not on screen): open it, show the cursor, and give input to game and UI
	if (!CraftWidget || !CraftWidget->IsInViewport())
	{
		if (!CraftWidget && CraftWidgetClass)
		{
			CraftWidget = CreateWidget<UUserWidget>(GetWorld(), CraftWidgetClass);
		}

		if (CraftWidget)
		{
			CraftWidget->AddToViewport();

			if (PC)
			{
				PC->bShowMouseCursor = true;
				FInputModeGameAndUI InputMode;
				InputMode.SetWidgetToFocus(CraftWidget->TakeWidget());
				PC->SetInputMode(InputMode);
			}
		}
	}
	// Next press: close it, hide the cursor, and hand input back to the game
	else
	{
		CraftWidget->RemoveFromParent();

		if (PC)
		{
			PC->bShowMouseCursor = false;
			FInputModeGameOnly InputMode;
			PC->SetInputMode(InputMode);
		}
	}
}
