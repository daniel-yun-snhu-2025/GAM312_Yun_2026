#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AICharacter.generated.h"

UCLASS()
class GAM312_YUN_2026_API AAICharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets the wander controller defaults - the mesh (Manny) is assigned on
	// the AIChar_BP child in Class Defaults, like in the video
	AAICharacter();
};
