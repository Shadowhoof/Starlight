// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StarlightCharacter.generated.h"

class UCapsuleComponent;
class UMotionControllerComponent;
class UCameraComponent;


UENUM(BlueprintType)
enum class EVRMovementType : uint8
{
	/** Aim with motion controllers to select a location to teleport to */
	Teleport,

	/** Continuous movement based on thumbstick directional input. Movement direction is based on where controllers are facing. */
	Continuous,
};


UCLASS()
class STARLIGHT_API AStarlightCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	
	AStarlightCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual void Tick(float DeltaSeconds) override;
	
protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionControls")
	TObjectPtr<UMotionControllerComponent> LeftController;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "MotionControls")
	TObjectPtr<UMotionControllerComponent> RightController;

	/** Yaw change in degrees whenever snap turn is performed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SnapTurnDegrees = 45.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	EVRMovementType VRMovementType = EVRMovementType::Continuous;

	TObjectPtr<APlayerController> PlayerController;
	
private:

	bool bIsTeleporting = false;

	float SnapTurnDegreesWithScale = 0.f;
	
private:

	static bool IsHMDActive();
	
	void LookUp(const float Rate);
	void LookRight(const float Rate);
	void SnapTurn(const float Sign);

	void MoveForward(const float Rate);
	void MoveRight(const float Rate);
	FVector GetMovementForwardVector() const;
	FVector GetMovementRightVector() const;
	
	void StartTeleport();
	void FinishTeleport();
	void CancelTeleport();
	void UpdateTeleport();
	void TraceTeleport(FHitResult& OutHit);

};
