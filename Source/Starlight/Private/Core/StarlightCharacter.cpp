
#include "Core/StarlightCharacter.h"

#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"
#include "IXRTrackingSystem.h"
#include "GameFramework/CharacterMovementComponent.h"


namespace TraceConstants
{
	const float TeleportTraceDistance = 100000.f;
	const float HitLocationOffset = 50.f;
}


AStarlightCharacter::AStarlightCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->bLockToHmd = true;
	CameraComponent->SetupAttachment(RootComponent);
	
	LeftController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("LeftController"));
	LeftController->SetTrackingSource(EControllerHand::Left);
	LeftController->SetupAttachment(RootComponent);
	
	RightController = CreateDefaultSubobject<UMotionControllerComponent>(TEXT("RightController"));
	FVector MirroredScale = RightController->GetRelativeScale3D();
	MirroredScale.Y *= -1;
	RightController->SetRelativeScale3D(MirroredScale);
	RightController->SetTrackingSource(EControllerHand::Right);
	RightController->SetupAttachment(RootComponent);
}

void AStarlightCharacter::BeginPlay()
{
	Super::BeginPlay();

	PlayerController = Cast<APlayerController>(GetController());
	if (!PlayerController)
	{
		UE_LOG(LogPlayerController, Error, TEXT("%s's controller is not a player controller"), *GetName())
		return;
	}
	
	if (!IsHMDActive())
	{
		CameraComponent->bUsePawnControlRotation = true;
		bUseControllerRotationYaw = true;
	}

	SnapTurnDegreesWithScale = SnapTurnDegrees / PlayerController->InputYawScale;
}

void AStarlightCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("LookUp", this, &AStarlightCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AStarlightCharacter::LookRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AStarlightCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AStarlightCharacter::MoveRight);

	PlayerInputComponent->BindAction("Teleport", IE_Pressed, this, &AStarlightCharacter::StartTeleport);
	PlayerInputComponent->BindAction("Teleport", IE_Released, this, &AStarlightCharacter::FinishTeleport);
	PlayerInputComponent->BindAction("CancelTeleport", IE_Pressed, this, &AStarlightCharacter::CancelTeleport);

	DECLARE_DELEGATE_OneParam(FSnapTurnDelegate, float)
	PlayerInputComponent->BindAction<FSnapTurnDelegate>("SnapTurnLeft", IE_Pressed, this, &AStarlightCharacter::SnapTurn, -1.f);
	PlayerInputComponent->BindAction<FSnapTurnDelegate>("SnapTurnRight", IE_Pressed, this, &AStarlightCharacter::SnapTurn, 1.f);
}

void AStarlightCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateTeleport();
}

bool AStarlightCharacter::IsHMDActive()
{
	return GEngine->XRSystem.IsValid() && GEngine->XRSystem->IsHeadTrackingAllowed();
}

void AStarlightCharacter::LookUp(const float Rate)
{
	AddControllerPitchInput(Rate);
}

void AStarlightCharacter::LookRight(const float Rate)
{
	AddControllerYawInput(Rate);
}

void AStarlightCharacter::SnapTurn(const float Sign)
{
	AddControllerYawInput(Sign * SnapTurnDegreesWithScale);			
}

void AStarlightCharacter::MoveForward(const float Rate)
{
	if (!IsHMDActive() || VRMovementType == EVRMovementType::Continuous)
	{
		AddMovementInput(GetMovementForwardVector(), Rate);
	}
}

void AStarlightCharacter::MoveRight(const float Rate)
{
	if (!IsHMDActive() || VRMovementType == EVRMovementType::Continuous)
	{
		AddMovementInput(GetMovementRightVector(), Rate);	
	}
}

FVector AStarlightCharacter::GetMovementForwardVector() const
{
	if (!IsHMDActive())
	{
		return GetActorForwardVector();
	}

	const FVector AverageDirection = (LeftController->GetForwardVector() + RightController->GetForwardVector()) / 2;
	return AverageDirection.GetSafeNormal2D();
}

FVector AStarlightCharacter::GetMovementRightVector() const
{
	if (!IsHMDActive())
	{
		return GetActorRightVector();
	}

	const FVector AverageDirection = (LeftController->GetRightVector() + RightController->GetRightVector()) / 2;
	return AverageDirection.GetSafeNormal2D();
}

void AStarlightCharacter::StartTeleport()
{
	if (!bIsTeleporting)
	{
		bIsTeleporting = true;
	}
}

void AStarlightCharacter::FinishTeleport()
{
	if (bIsTeleporting)
	{
		FHitResult HitResult;
		TraceTeleport(HitResult);
		if (HitResult.IsValidBlockingHit())
		{
			FRotator DestDirection = (HitResult.Location - GetActorLocation()).Rotation();
			DestDirection.Pitch = 0.f;
			TeleportTo(HitResult.Location, DestDirection);
		}
		
		bIsTeleporting = false;
	}
}

void AStarlightCharacter::CancelTeleport()
{
	if (bIsTeleporting)
	{
		bIsTeleporting = false;
	}
}

void AStarlightCharacter::UpdateTeleport()
{
	if (!bIsTeleporting)
	{
		return;
	}

	FHitResult HitResult;
	TraceTeleport(HitResult);
	if (HitResult.IsValidBlockingHit())
	{
		DrawDebugSphere(GetWorld(), HitResult.Location, 50.f, 16, FColor::Red);
	}
}

void AStarlightCharacter::TraceTeleport(FHitResult& OutHit)
{
	const FVector StartPoint = IsHMDActive() ? LeftController->GetComponentLocation() : CameraComponent->GetComponentLocation();
	const FVector Direction = IsHMDActive() ? LeftController->GetComponentRotation().Vector() : CameraComponent->GetComponentRotation().Vector();
	const FVector EndPoint = StartPoint + Direction * TraceConstants::TeleportTraceDistance;
	
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	GetWorld()->LineTraceSingleByChannel(OutHit, StartPoint, EndPoint, ECC_WorldStatic, QueryParams);

	if (OutHit.IsValidBlockingHit())
	{
		OutHit.Location += OutHit.Normal * TraceConstants::HitLocationOffset;
	}
}

