// Shadowhoof Games, 2022


#include "StarlightCharacterMovementComponent.h"

#include "Core/StarlightCharacter.h"


UStarlightCharacterMovementComponent::UStarlightCharacterMovementComponent()
{
	bEnableScopedMovementUpdates = true;
}

void UStarlightCharacterMovementComponent::BeginPlay()
{
	Super::BeginPlay();

	Character = Cast<AStarlightCharacter>(GetCharacterOwner());
	ensureMsgf(Character, TEXT("UStarlightCharacterMovementComponent can only be used with AStarlightCharacter, current owner: %s"), *GetCharacterOwner()->GetName());
}

void UStarlightCharacterMovementComponent::OnMovementUpdated(float DeltaSeconds, const FVector& OldLocation,
                                                             const FVector& OldVelocity)
{
	Super::OnMovementUpdated(DeltaSeconds, OldLocation, OldVelocity);
	Character->OnTeleportableMoved();
}
