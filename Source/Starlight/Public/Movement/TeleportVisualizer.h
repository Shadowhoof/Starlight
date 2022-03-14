// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "TeleportVisualizer.generated.h"

UCLASS()
class STARLIGHT_API ATeleportVisualizer : public AActor
{
	GENERATED_BODY()

public:
	
	ATeleportVisualizer();

	void UpdateVisualizerLocation(const FVector& NewLocation);
	
protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "StaticMesh")
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;
	
};
