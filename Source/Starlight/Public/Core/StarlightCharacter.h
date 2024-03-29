// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Portal/Teleportable.h"
#include "StarlightCharacter.generated.h"

class IGrabbable;
enum class EPortalType : uint8;
class UPortalComponent;
class UTeleportComponent;
class UCapsuleComponent;
class UMotionControllerComponent;
class UCameraComponent;
class USphereComponent;
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
class STARLIGHT_API AStarlightCharacter : public ACharacter, public ITeleportable
{
	GENERATED_BODY()

public:
	AStarlightCharacter(const FObjectInitializer& ObjectInitializer);

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

	virtual void Tick(float DeltaSeconds) override;

	void OnObjectGrabbed(TObjectPtr<IGrabbable> Grabbable);
	void OnObjectReleased(TObjectPtr<IGrabbable> Grabbable);
	
	// Teleportable interface begin

	virtual void Teleport(TObjectPtr<APortal> SourcePortal, TObjectPtr<APortal> TargetPortal) override;

	virtual void OnOverlapWithPortalBegin(TObjectPtr<APortal> Portal) override;
	virtual void OnOverlapWithPortalEnd(TObjectPtr<APortal> Portal) override;

	virtual TObjectPtr<UPrimitiveComponent> GetCollisionComponent() const override;

	virtual void GetTeleportVelocity(FVector& LinearVelocity, FVector& AngularVelocity) const override;
	virtual void SetTeleportVelocity(const FVector& LinearVelocity, const FVector& AngularVelocity) override;

	virtual FVector GetTeleportableObjectLocation() const override;
	
	virtual void OnTeleportableMoved() override;

	virtual TSubclassOf<ATeleportableCopy> GetPortalCopyClass() const override;

	virtual ECollisionChannel GetTeleportableBaseObjectType() override;

	// Teleportable interface end

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

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Portal")
	TObjectPtr<UPortalComponent> PortalComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Collision")
	TObjectPtr<USphereComponent> HeldObjectCollisionComponent;
	
	/** Yaw change in degrees whenever snap turn is performed */
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	float SnapTurnDegrees = 45.f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Movement")
	EMovementType MovementType = EMovementType::Continuous;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Portal")
	TSubclassOf<ATeleportableCopy> PortalCopyClass;
	
	UPROPERTY(Transient)
	TObjectPtr<APlayerController> PlayerController;

private:
	UPROPERTY(Transient)
	TMap<EControllerHand, TObjectPtr<UGrabDevice>> GrabDevices;

	UPROPERTY()
	TArray<TObjectPtr<APortal>> OverlappingPortals;

	FQuat PostTeleportInitialQuat;
	float PostTeleportRotationProgress = 0.f;
	bool bIsPostTeleportRotation = false;

private:
	void LookUp(const float Rate);
	void LookRight(const float Rate);
	void SnapTurn(const float Sign);

	void MoveForward(const float Rate);
	void MoveRight(const float Rate);

	void Grab(EControllerHand Hand);
	void ReleaseGrab(EControllerHand Hand);

	void ShootPortal(EPortalType PortalType);

	void IntepolateRotation(const float DeltaSeconds);
};
