// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Grab/Grabbable.h"
#include "Portal/Teleportable.h"
#include "StarlightActor.generated.h"

/**
 *	Static mesh actor which can be grabbed and is able to teleport through portals. 
 */
UCLASS()
class STARLIGHT_API AStarlightActor : public AActor, public ITeleportable, public IGrabbable
{
	GENERATED_BODY()

public:
	
	AStarlightActor();

	virtual TObjectPtr<UPrimitiveComponent> GetAttachComponent() const override;

	virtual FRotator GetTeleportRotation() override;
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StaticMesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

protected:

	virtual bool SetTeleportLocationAndRotation(const FVector& Location, const FRotator& Rotation) override;
	
	virtual TObjectPtr<UPrimitiveComponent> GetCollisionComponent() const override;

	virtual FVector GetVelocity() const override;
	virtual void SetVelocity(const FVector& Velocity) override;

};
