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
	APlayerChar();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION()
	void MoveForward(float axisValue);

	UFUNCTION()
	void MoveRight(float axisValue);

	UFUNCTION()
	void StartJump();

	UFUNCTION()
	void StopJump();

	UFUNCTION()
	void FindObject();

	UPROPERTY(VisibleAnywhere)
	UCameraComponent* PlayerCamComp;
};
