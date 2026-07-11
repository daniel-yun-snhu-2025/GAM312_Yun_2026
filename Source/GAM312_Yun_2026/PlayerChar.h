#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
// Included so we can cast line trace hits to our master resource class
#include "Resource_M.h"
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

	// Player stat variables - editable in the Details panel and readable/writable from Blueprints,
	// grouped under the Player Stats category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Health = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Hunger = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Player Stats")
	float Stamina = 100.0f;

	// Blueprint Callable events for adjusting the player stats - a positive amount adds to
	// the stat and a negative amount subtracts from it
	UFUNCTION(BlueprintCallable)
	void SetHealth(float amount);

	UFUNCTION(BlueprintCallable)
	void SetHunger(float amount);

	UFUNCTION(BlueprintCallable)
	void SetStamina(float amount);

	// Called on a timer - decreases Hunger over time, regenerates Stamina,
	// and drains Health once Hunger reaches zero
	UFUNCTION()
	void DecreaseStats();

	// Adds the collected amount to the matching index of the resources array
	UFUNCTION()
	void GiveResource(float amount, FString resourceType);

	// Timer handle driving the DecreaseStats interval
	FTimerHandle StatsTimerHandle;

	// Individual resource counts, grouped under the Resources category
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	int Wood;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	int Stone;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Resources")
	int Berry;

	// Overall resources array - index 0 is Wood, 1 is Stone, 2 is Berry
	UPROPERTY(EditAnywhere, Category = "Resources")
	TArray<int> ResourcesArray;

	// Names matching the resources array indices - these must line up with ResourcesArray
	UPROPERTY(EditAnywhere, Category = "Resources")
	TArray<FString> ResourcesNameArray;
};
