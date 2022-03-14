// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "StarlightCharacter.generated.h"

class UTeleportComponent;
class UCapsuleComponent;
class UMotionControllerComponent;
class UCameraComponent;
class UGrabDevice;


UENUM(BlueprintType)
enum class EMovementType : uint8
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

	TObjectPtr<APlayerController> GetPlayerController() const;

	TObjectPtr<UCameraComponent> GetCameraComponent() const;

	/**
	 *	Gets forward vector that can be used for movement. With motion controllers movement forward vector
	 *	can differ from actor forward vector.
	 */
	FVector GetMovementForwardVector() const;

	/**
	 *	Gets right vector that can be used for movement. With motion controllers movement right vector
	 *	can differ from actor right vector.
	 */
	FVector GetMovementRightVector() const;
	
protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionControls")
	TObjectPtr<UMotionControllerComponent> LeftController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "MotionControls")
	TObjectPtr<UMotionControllerComponent> RightController;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	TObjectPtr<UTeleportComponent> TeleportComponent;
	
	/** Yaw change in degrees whenever snap turn is performed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SnapTurnDegrees = 45.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	EMovementType MovementType = EMovementType::Continuous;

	UPROPERTY(Transient)
	TObjectPtr<APlayerController> PlayerController;
	
private:
	
	UPROPERTY(Transient)
	TMap<EControllerHand, TObjectPtr<UGrabDevice>> GrabDevices;
	
private:

	void LookUp(const float Rate);
	void LookRight(const float Rate);
	void SnapTurn(const float Sign);

	void MoveForward(const float Rate);
	void MoveRight(const float Rate);

	void Grab(EControllerHand Hand);
	void ReleaseGrab(EControllerHand Hand);
};
