#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "PlayerChar.generated.h"

class UCameraComponent;

UCLASS()
class GAM312_YUN_2026_API APlayerChar : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	APlayerChar();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Establishing the functions that we are going to be utilizing for the character's movement
	// Forward and backward movement, receives the axis value from the MoveForward input (+1 = W, -1 = S)
	UFUNCTION()
	void MoveForward(float axisValue);

	// Left and right movement, receives the axis value from the MoveRight input (-1 = A, +1 = D)
	UFUNCTION()
	void MoveRight(float axisValue);

	// Custom jump events - set and clear the jump flag used by the built-in Character jump
	UFUNCTION()
	void StartJump();

	UFUNCTION()
	void StopJump();

	// FindObject event - will run the line trace for detecting objects when Interact is pressed
	UFUNCTION()
	void FindObject();

	// Establishing the camera component so we can access the camera in code and attach it to the head
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* PlayerCamComp;
};
