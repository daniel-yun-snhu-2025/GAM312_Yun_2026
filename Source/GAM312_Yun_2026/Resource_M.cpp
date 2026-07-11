#include "Resource_M.h"

// Sets default values
AResource_M::AResource_M()
{
	// Set this actor to call Tick() every frame
	PrimaryActorTick.bCanEverTick = true;

	// Creating the text render and mesh components
	ResourceNameText = CreateDefaultSubobject<UTextRenderComponent>(TEXT("Resource Name Text"));
	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));

	// The mesh is the root, with the name text attached to it
	RootComponent = Mesh;
	ResourceNameText->SetupAttachment(Mesh);
}

// Called when the game starts or when spawned
void AResource_M::BeginPlay()
{
	Super::BeginPlay();

	// Display whatever resourceName was set in the Details panel above the mesh
	tempText = FText::FromString(resourceName);
	ResourceNameText->SetText(tempText);
}

// Called every frame
void AResource_M::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
