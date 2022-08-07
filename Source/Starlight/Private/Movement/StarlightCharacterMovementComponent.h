// Shadowhoof Games, 2022

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "StarlightCharacterMovementComponent.generated.h"

class AStarlightCharacter;


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class STARLIGHT_API UStarlightCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:

	UStarlightCharacterMovementComponent();
	
protected:

	virtual void BeginPlay() override;
	
	virtual void OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation, const FVector& OldVelocity) override;

private:

	UPROPERTY(Transient)
	TObjectPtr<AStarlightCharacter> Character;
	
};
