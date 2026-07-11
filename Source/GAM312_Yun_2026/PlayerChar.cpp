#include "PlayerChar.h"
#include "Camera/CameraComponent.h"

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
