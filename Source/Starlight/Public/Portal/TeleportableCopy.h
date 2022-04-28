// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeleportableCopy.generated.h"

class APortal;
class AStarlightActor;
enum class EPortalType : uint8;


UCLASS()
class STARLIGHT_API ATeleportableCopy : public AActor
{
	GENERATED_BODY()

public:
	
	ATeleportableCopy();

	// TODO - support actors with skeletal meshes
	/**
	 * Initializes created teleportable actor copy
	 * @param InParent Actor that served as a base for this copy
	 * @param StaticMesh Parent actor's mesh
	 * @param PortalType Type of portal that created and owns this copy
	 */
	void Initialize(TObjectPtr<AActor> InParent, TObjectPtr<UStaticMesh> StaticMesh, EPortalType PortalType);

	/**
	 * Sets up culling parameters for dynamic material
	 */
	void UpdateCullingParams(const FVector& CullPlaneCenter, const FVector& CullPlaneNormal);
	
	TObjectPtr<AActor> GetParent() const;
	
protected:

	UPROPERTY()
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY()
	TObjectPtr<AActor> Parent;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> DynamicMaterialInstance;
	
};
