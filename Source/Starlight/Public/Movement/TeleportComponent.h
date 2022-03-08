// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "TeleportComponent.generated.h"


class UMotionControllerComponent;
struct FPredictProjectilePathResult;
class AStarlightCharacter;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STARLIGHT_API UTeleportComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	
	UTeleportComponent();

	void MoveYAxis(const float Rate);

	void CancelTeleport();
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
						   FActorComponentTickFunction* ThisTickFunction) override;

	void SetTeleportController(TObjectPtr<UMotionControllerComponent> Controller);
	
protected:
	
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Teleport")
	float TeleportActivationAxisThreshold = 0.7f;
	
private:

	UPROPERTY()
	TObjectPtr<AStarlightCharacter> OwnerCharacter;

	UPROPERTY()
	TObjectPtr<UMotionControllerComponent> TeleportController;
	
	bool bIsTeleporting = false;
	bool bIsTeleportAxisOverThreshold = false;

	float ProjectileRadius = 50.f;
	
	void StartTeleport();
	void FinishTeleport();
	void UpdateTeleport(FPredictProjectilePathResult& OutPredictResult);
	
	
};
