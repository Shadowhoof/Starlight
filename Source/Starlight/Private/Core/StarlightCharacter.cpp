#include "Core/StarlightCharacter.h"

#include "Components/CapsuleComponent.h"
#include "MotionControllerComponent.h"
#include "Camera/CameraComponent.h"
#include "IXRTrackingSystem.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Grab/MotionControllerGrabDevice.h"
#include "Grab/TraceGrabDevice.h"
#include "Movement/TeleportComponent.h"
#include "Portal/Portal.h"
#include "Portal/PortalConstants.h"
#include "Portal/PortalComponent.h"
#include "Statics/StarlightStatics.h"


namespace Constants
{
	const float RollUpdateRate = 360.f;		/* Roll change per second if it's different from 0.0 */
	const float PitchUpdateRate = 360.f;	/* Pitch change per second for actor if it's different from 0.0 */
}


AStarlightCharacter::AStarlightCharacter()
{
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

	TeleportComponent = CreateDefaultSubobject<UTeleportComponent>(TEXT("TeleportComponent"));
	PortalComponent = CreateDefaultSubobject<UPortalComponent>(TEXT("PortalComponent"));
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
	
	if (UStarlightStatics::IsHMDActive())
	{
		TeleportComponent->SetTeleportController(LeftController);
	}
	else
	{
		CameraComponent->bUsePawnControlRotation = true;
		bUseControllerRotationYaw = true;
		LeftController->SetHiddenInGame(true, true);
		RightController->SetHiddenInGame(true, true);
	}

	// initialize grab devices
	if (UStarlightStatics::IsHMDActive())
	{
		UMotionControllerGrabDevice* LeftGrabDevice = NewObject<UMotionControllerGrabDevice>(this);
		LeftGrabDevice->Initialize(LeftController);
		GrabDevices.Add(EControllerHand::Left, LeftGrabDevice);

		UMotionControllerGrabDevice* RightGrabDevice = NewObject<UMotionControllerGrabDevice>(this);
		RightGrabDevice->Initialize(RightController);
		GrabDevices.Add(EControllerHand::Right, RightGrabDevice);
	}
	else
	{
		UTraceGrabDevice* TraceGrabDevice = NewObject<UTraceGrabDevice>(this);
		TraceGrabDevice->Initialize(CameraComponent);
		GrabDevices.Add(EControllerHand::Special_1, TraceGrabDevice);
	}

	OnCharacterMovementUpdated.AddDynamic(this, &AStarlightCharacter::OnMovement);
}

void AStarlightCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("LookUp", this, &AStarlightCharacter::LookUp);
	PlayerInputComponent->BindAxis("LookRight", this, &AStarlightCharacter::LookRight);
	PlayerInputComponent->BindAxis("MoveForward", this, &AStarlightCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AStarlightCharacter::MoveRight);

	DECLARE_DELEGATE_OneParam(FGrabDelegate, EControllerHand)
	PlayerInputComponent->BindAction<FGrabDelegate>("GrabLeft", IE_Pressed, this, &AStarlightCharacter::Grab,
	                                                EControllerHand::Left);
	PlayerInputComponent->BindAction<FGrabDelegate>("GrabLeft", IE_Released, this, &AStarlightCharacter::ReleaseGrab,
	                                                EControllerHand::Left);
	PlayerInputComponent->BindAction<FGrabDelegate>("GrabRight", IE_Pressed, this, &AStarlightCharacter::Grab,
	                                                EControllerHand::Right);
	PlayerInputComponent->BindAction<FGrabDelegate>("GrabRight", IE_Released, this, &AStarlightCharacter::ReleaseGrab,
	                                                EControllerHand::Right);
	PlayerInputComponent->BindAction<FGrabDelegate>("GrabSpecial", IE_Pressed, this, &AStarlightCharacter::Grab,
	                                                EControllerHand::Special_1);
	PlayerInputComponent->BindAction<FGrabDelegate>("GrabSpecial", IE_Released, this, &AStarlightCharacter::ReleaseGrab,
	                                                EControllerHand::Special_1);

	DECLARE_DELEGATE_OneParam(FSnapTurnDelegate, float)
	PlayerInputComponent->BindAction<FSnapTurnDelegate>("SnapTurnLeft", IE_Pressed, this,
	                                                    &AStarlightCharacter::SnapTurn, -1.f);
	PlayerInputComponent->BindAction<FSnapTurnDelegate>("SnapTurnRight", IE_Pressed, this,
	                                                    &AStarlightCharacter::SnapTurn, 1.f);

	DECLARE_DELEGATE_OneParam(FShootPortalDelegate, EPortalType)
	PlayerInputComponent->BindAction<FShootPortalDelegate>("ShootPortalOne", IE_Pressed, this,
	                                                       &AStarlightCharacter::ShootPortal, EPortalType::First);
	PlayerInputComponent->BindAction<FShootPortalDelegate>("ShootPortalTwo", IE_Pressed, this,
	                                                       &AStarlightCharacter::ShootPortal, EPortalType::Second);
}

TObjectPtr<APlayerController> AStarlightCharacter::GetPlayerController() const
{
	return PlayerController;
}

TObjectPtr<UCameraComponent> AStarlightCharacter::GetCameraComponent() const
{
	return CameraComponent;
}

TObjectPtr<UPrimitiveComponent> AStarlightCharacter::GetCollisionComponent() const
{
	return GetCapsuleComponent();
}

void AStarlightCharacter::GetTeleportVelocity(FVector& LinearVelocity, FVector& AngularVelocity) const
{
	LinearVelocity = GetCharacterMovement()->Velocity;
	AngularVelocity = FVector::ZeroVector;
}

void AStarlightCharacter::SetTeleportVelocity(const FVector& LinearVelocity, const FVector& AngularVelocity)
{
	GetCharacterMovement()->Velocity = LinearVelocity;
}

void AStarlightCharacter::OnTeleportableMoved()
{
	for (APortal* Portal : OverlappingPortals)
	{
		Portal->OnActorMoved(this);
	}
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
	AddControllerYawInput(Sign * SnapTurnDegrees);
}

void AStarlightCharacter::MoveForward(const float Rate)
{
	switch (MovementType)
	{
	case EMovementType::Continuous:
		AddMovementInput(GetMovementForwardVector(), Rate);
		break;
	case EMovementType::Teleport:
		TeleportComponent->MoveYAxis(Rate);
		break;
	default:
		break;
	}
}

void AStarlightCharacter::MoveRight(const float Rate)
{
	if (MovementType == EMovementType::Continuous)
	{
		AddMovementInput(GetMovementRightVector(), Rate);
	}
}

FVector AStarlightCharacter::GetMovementForwardVector() const
{
	if (!UStarlightStatics::IsHMDActive())
	{
		return GetActorForwardVector();
	}

	const FVector AverageDirection = (LeftController->GetForwardVector() + RightController->GetForwardVector()) / 2;
	return AverageDirection.GetSafeNormal2D();
}

FVector AStarlightCharacter::GetMovementRightVector() const
{
	if (!UStarlightStatics::IsHMDActive())
	{
		return GetActorRightVector();
	}

	const FVector AverageDirection = (LeftController->GetRightVector() + RightController->GetRightVector()) / 2;
	return AverageDirection.GetSafeNormal2D();
}

void AStarlightCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateRotation(DeltaSeconds);

	for (const auto& Entry : GrabDevices)
	{
		Entry.Value->Tick(DeltaSeconds);
	}
}

void AStarlightCharacter::Teleport(TObjectPtr<APortal> SourcePortal, TObjectPtr<APortal> TargetPortal)
{
	const FRotator NewControlRotation = SourcePortal->TeleportRotation(GetControlRotation());
	ITeleportable::Teleport(SourcePortal, TargetPortal);
	GetController()->SetControlRotation(NewControlRotation);
}

void AStarlightCharacter::OnOverlapWithPortalBegin(TObjectPtr<APortal> Portal)
{
	ensure(!OverlappingPortals.Contains(Portal));
	ITeleportable::OnOverlapWithPortalBegin(Portal);
	OverlappingPortals.Add(Portal);
}

void AStarlightCharacter::OnOverlapWithPortalEnd(TObjectPtr<APortal> Portal)
{
	ensure(OverlappingPortals.Contains(Portal));
	ITeleportable::OnOverlapWithPortalEnd(Portal);
	OverlappingPortals.Remove(Portal);
}

void AStarlightCharacter::Grab(EControllerHand Hand)
{
	TObjectPtr<UGrabDevice>* GrabDevicePtr = GrabDevices.Find(Hand);
	if (GrabDevicePtr)
	{
		GrabDevicePtr->Get()->TryGrabbing();
	}
}

void AStarlightCharacter::ReleaseGrab(EControllerHand Hand)
{
	TObjectPtr<UGrabDevice>* GrabDevicePtr = GrabDevices.Find(Hand);
	if (GrabDevicePtr)
	{
		GrabDevicePtr->Get()->Release();
	}
}

void AStarlightCharacter::ShootPortal(EPortalType PortalType)
{
	FVector EyesLocation;
	FRotator EyesRotation;
	GetActorEyesViewPoint(EyesLocation, EyesRotation);
	PortalComponent->ShootPortal(PortalType, EyesLocation, EyesRotation.Vector());
}

void AStarlightCharacter::OnMovement(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	OnTeleportableMoved();
}

void AStarlightCharacter::UpdateRotation(const float DeltaSeconds)
{
	FRotator ActorRotation = GetActorRotation();
	if (!FMath::IsNearlyZero(ActorRotation.Pitch) || !FMath::IsNearlyZero(ActorRotation.Roll))
	{
		ActorRotation.Pitch = GetUpdatedAngle(DeltaSeconds, ActorRotation.Pitch, Constants::PitchUpdateRate);
		ActorRotation.Roll = GetUpdatedAngle(DeltaSeconds, ActorRotation.Roll, Constants::RollUpdateRate);
		SetActorRotation(ActorRotation);
	}

	FRotator ControlRotation = GetControlRotation();
	if (!FMath::IsNearlyZero(ControlRotation.Roll))
	{
		ControlRotation.Roll = GetUpdatedAngle(DeltaSeconds, ControlRotation.Roll, Constants::RollUpdateRate);
		GetController()->SetControlRotation(ControlRotation);
	}
}

float AStarlightCharacter::GetUpdatedAngle(const float DeltaSeconds, float InitialAngle, const float UpdateRate)
{
	InitialAngle = FRotator::NormalizeAxis(InitialAngle);
	const float DeltaAngle = DeltaSeconds * Constants::RollUpdateRate;
	return InitialAngle > 0.f
		       ? FMath::Max(InitialAngle - DeltaAngle, 0.f)
		       : FMath::Min(InitialAngle + DeltaAngle, 0.f);
}
