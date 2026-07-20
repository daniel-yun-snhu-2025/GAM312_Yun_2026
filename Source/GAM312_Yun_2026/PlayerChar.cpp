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

	// Initial setup of the camera component
	PlayerCamComp = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));

	// Attaching the camera to the character mesh and the head bone
	PlayerCamComp->SetupAttachment(GetMesh(), "head");

	// Using the controller's rotation for the camera - the pawn shares the controller's
	// rotation, so turning the camera turns the character along with it (true first person)
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
}

// Called every frame
void APlayerChar::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// While placing a building part, keep it ahead of the camera so it follows
	// wherever the player points until they click to place it
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
					FVector PartOrigin;
					FVector PartExtent;
					HitPart->GetActorBounds(true, PartOrigin, PartExtent);

					if (spawnedPartID == 1)
					{
						// Floor - drop to the hit part's bottom so the plate tucks under it
						EndLocation.Z = PartOrigin.Z - PartExtent.Z;
					}
					else
					{
						// Wall or ceiling - snap onto the hit part's top
						EndLocation.Z = PartOrigin.Z + PartExtent.Z;
					}

					spawnedPart->SetActorLocation(EndLocation);
					return;
				}
			}

			// Otherwise drop straight down from just above the aim point so the
			// pivot rests level on the supporting surface
			FHitResult GroundHit;
			if (GetWorld()->LineTraceSingleByChannel(GroundHit, EndLocation + FVector(0.0f, 0.0f, 50.0f), EndLocation - FVector(0.0f, 0.0f, 2000.0f), ECC_Visibility, TraceParams))
			{
				EndLocation.Z = GroundHit.Location.Z;
			}

			spawnedPart->SetActorLocation(EndLocation);
		}
	}
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
		return;
	}

	// Line trace setup - fires from the camera to whatever it is looking at
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

	// Once Hunger is empty, Health starts draining instead
	if (Hunger <= 0)
	{
		SetHealth(-1.0f);
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
