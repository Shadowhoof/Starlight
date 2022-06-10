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

	// Grabbable interface begin
	
	virtual TObjectPtr<UPrimitiveComponent> GetComponentToGrab() const override;

	virtual void OnGrab() override;
	virtual void OnRelease() override;
	
	virtual bool IsGrabbed() const override;

	virtual void OnGrabbableMoved(const float Speed) override;

	// Grabbable interface end 

	// Teleportable interface begin

	virtual void OnOverlapWithPortalBegin(TObjectPtr<APortal> Portal) override;
	virtual void OnOverlapWithPortalEnd(TObjectPtr<APortal> Portal) override;

	virtual void Tick(float DeltaSeconds) override;

	virtual TSubclassOf<ATeleportableCopy> GetPortalCopyClass() const override;
	
	virtual TObjectPtr<UPrimitiveComponent> GetCollisionComponent() const override;

	virtual void GetTeleportVelocity(FVector& LinearVelocity, FVector& AngularVelocity) const override;
	virtual void SetTeleportVelocity(const FVector& LinearVelocity, const FVector& AngularVelocity) override;

	virtual ECollisionChannel GetTeleportableBaseObjectType() override;

	// Teleportable interface end
	
	virtual void NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved,
	                       FVector HitLocation, FVector HitNormal, FVector NormalImpulse,
	                       const FHitResult& Hit) override;

protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StaticMesh")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterialInstance;

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TArray<TObjectPtr<APortal>> OverlappingPortals;

	bool bIsGrabbed = false;

	float GrabbedMovementSpeed = 0.f;

private:
	void UpdateMaterialParameters();
};
