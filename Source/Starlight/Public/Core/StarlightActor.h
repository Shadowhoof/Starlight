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

	virtual void OnOverlapWithPortalBegin(TObjectPtr<APortal> Portal) override;
	virtual void OnOverlapWithPortalEnd(TObjectPtr<APortal> Portal) override;

	virtual void Tick(float DeltaSeconds) override;

	virtual TObjectPtr<ATeleportableCopy> CreatePortalCopy(const FTransform& SpawnTransform, TObjectPtr<APortal> Portal, TObjectPtr<AActor> ParentActor) override;
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StaticMesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterialInstance;

protected:

	virtual void BeginPlay() override;
	
	virtual TObjectPtr<UPrimitiveComponent> GetCollisionComponent() const override;

	virtual FVector GetVelocity() const override;
	virtual void SetVelocity(const FVector& Velocity) override;

private:

	UPROPERTY()
	TArray<TObjectPtr<APortal>> OverlappingPortals;
	
private:

	void UpdateMaterialParameters();

};
