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

	// Display the object name (Tree, Rock, Bush) above the mesh
	tempText = FText::FromString(objectName);
	ResourceNameText->SetText(tempText);
}

// Called in the editor when the actor is placed or edited - keeps the label
// showing the object name in the viewport without needing to play
void AResource_M::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	ResourceNameText->SetText(FText::FromString(objectName));
}

// Called every frame
void AResource_M::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
