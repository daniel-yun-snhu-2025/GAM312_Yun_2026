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
}

// Called when the game starts or when spawned
void APlayerChar::BeginPlay()
{
	Super::BeginPlay();
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
	// Line trace for detecting objects will be established here in a later stepping stone
}
