// Shadowhoof Games, 2022


#include "Movement/TeleportComponent.h"

#include "DrawDebugHelpers.h"
#include "MotionControllerComponent.h"
#include "Components/CapsuleComponent.h"
#include "Core/StarlightCharacter.h"
#include "Kismet/GameplayStatics.h"


namespace TeleportConstants
{
	const float ProjectileLaunchSpeed = 700.f;
	const float MaxSimulationTime = 1.f;
	const FVector TraceStartOffset = {0.f, -30.f, -50.f};
}


UTeleportComponent::UTeleportComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UTeleportComponent::MoveYAxis(const float Rate)
{
	if (Rate > TeleportActivationAxisThreshold)
	{
		if (!bIsTeleportAxisOverThreshold)
		{
			StartTeleport();
			bIsTeleportAxisOverThreshold = true;
		}
	}
	else
	{
		bIsTeleportAxisOverThreshold = false;
		FinishTeleport();
	}
}

void UTeleportComponent::CancelTeleport()
{
	if (bIsTeleporting)
	{
		bIsTeleporting = false;
	}
}

void UTeleportComponent::BeginPlay()
{
	Super::BeginPlay();

	OwnerCharacter = Cast<AStarlightCharacter>(GetOwner());
	if (OwnerCharacter)
	{
		ProjectileRadius = OwnerCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();		
	}
}

void UTeleportComponent::StartTeleport()
{
	if (!bIsTeleporting)
	{
		bIsTeleporting = true;
	}
}

void UTeleportComponent::FinishTeleport()
{
	if (bIsTeleporting)
	{
		FPredictProjectilePathResult PredictResult;
		UpdateTeleport(PredictResult);
		if (PredictResult.HitResult.IsValidBlockingHit())
		{
			OwnerCharacter->TeleportTo(PredictResult.HitResult.Location, OwnerCharacter->GetActorRotation());
		}
		
		bIsTeleporting = false;
	}
}

void UTeleportComponent::UpdateTeleport(FPredictProjectilePathResult& OutPredictResult)
{
	const FVector TeleportDirection = TeleportController ? TeleportController->GetComponentRotation().Vector() : OwnerCharacter->GetControlRotation().Vector(); 
	const FVector LaunchVelocity = TeleportDirection * TeleportConstants::ProjectileLaunchSpeed;
	const FVector StartLocation = TeleportController ? TeleportController->GetComponentLocation() : OwnerCharacter->GetActorLocation();
	const FPredictProjectilePathParams PredictParams = {
		ProjectileRadius,
		StartLocation,
		LaunchVelocity,
		TeleportConstants::MaxSimulationTime,
		ECC_WorldStatic,
		OwnerCharacter
	};
	UGameplayStatics::PredictProjectilePath(this, PredictParams, OutPredictResult);
}

void UTeleportComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (bIsTeleporting)
	{
		FPredictProjectilePathResult PredictResult;
		UpdateTeleport(PredictResult);
		if (PredictResult.HitResult.IsValidBlockingHit())
		{
			DrawDebugSphere(GetWorld(), PredictResult.HitResult.Location, 50, 16, FColor::Red);
		}

		const TArray<FPredictProjectilePathPointData>& PathData = PredictResult.PathData;
		UE_LOG(LogTemp, Log, TEXT("path points: %d, blocking hit: %s, actor: %s"), PathData.Num(), PredictResult.HitResult.bBlockingHit ? TEXT("true") : TEXT("false"),
			PredictResult.HitResult.GetActor() ? *PredictResult.HitResult.GetActor()->GetName() : TEXT("Null"));
		for (int32 i = 0; i < PathData.Num() - 1; ++i)
		{
			const FVector Offset = OwnerCharacter->GetActorRightVector() * 100.f;
			DrawDebugLine(GetWorld(), PathData[i].Location, PathData[i + 1].Location, FColor::Purple);
		}
	}
}

void UTeleportComponent::SetTeleportController(TObjectPtr<UMotionControllerComponent> Controller)
{
	TeleportController = Controller;
}

