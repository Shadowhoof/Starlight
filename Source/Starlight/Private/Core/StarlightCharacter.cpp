
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


namespace TeleportConstants
{
	const float TeleportTraceDistance = 100000.f;
	const float HitLocationOffset = 50.f;
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
		TraceGrabDevice->Initialize(this);
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
	PlayerInputComponent->BindAction<FGrabDelegate>("GrabLeft", IE_Pressed, this, &AStarlightCharacter::Grab, EControllerHand::Left);
	PlayerInputComponent->BindAction<FGrabDelegate>("GrabLeft", IE_Released, this, &AStarlightCharacter::ReleaseGrab, EControllerHand::Left);
	PlayerInputComponent->BindAction<FGrabDelegate>("GrabRight", IE_Pressed, this, &AStarlightCharacter::Grab, EControllerHand::Right);
	PlayerInputComponent->BindAction<FGrabDelegate>("GrabRight", IE_Released, this, &AStarlightCharacter::ReleaseGrab, EControllerHand::Right);
	PlayerInputComponent->BindAction<FGrabDelegate>("GrabSpecial", IE_Pressed, this, &AStarlightCharacter::Grab, EControllerHand::Special_1);
	PlayerInputComponent->BindAction<FGrabDelegate>("GrabSpecial", IE_Released, this, &AStarlightCharacter::ReleaseGrab, EControllerHand::Special_1);

	DECLARE_DELEGATE_OneParam(FSnapTurnDelegate, float)
	PlayerInputComponent->BindAction<FSnapTurnDelegate>("SnapTurnLeft", IE_Pressed, this, &AStarlightCharacter::SnapTurn, -1.f);
	PlayerInputComponent->BindAction<FSnapTurnDelegate>("SnapTurnRight", IE_Pressed, this, &AStarlightCharacter::SnapTurn, 1.f);

	DECLARE_DELEGATE_OneParam(FShootPortalDelegate, EPortalType)
	PlayerInputComponent->BindAction<FShootPortalDelegate>("ShootPortalOne", IE_Pressed, this, &AStarlightCharacter::ShootPortal, EPortalType::First);
	PlayerInputComponent->BindAction<FShootPortalDelegate>("ShootPortalTwo", IE_Pressed, this, &AStarlightCharacter::ShootPortal, EPortalType::Second);
}

TObjectPtr<APlayerController> AStarlightCharacter::GetPlayerController() const
{
	return PlayerController;
}

TObjectPtr<UCameraComponent> AStarlightCharacter::GetCameraComponent() const
{
	return CameraComponent;
}

bool AStarlightCharacter::SetTeleportLocationAndRotation(const FVector& Location, const FRotator& Rotation)
{
	const FRotator NewActorRotation = FRotator(0.f, Rotation.Yaw, 0.f);
	if (!TeleportTo(Location, NewActorRotation))
	{
		return false;
	}

	const FRotator NewControlRotation = FRotator(Rotation.Pitch, Rotation.Yaw, 0.f);
	GetController()->SetControlRotation(NewControlRotation);
	return true;
}

TObjectPtr<UPrimitiveComponent> AStarlightCharacter::GetCollisionComponent() const
{
	return GetCapsuleComponent();
}

FVector AStarlightCharacter::GetVelocity() const
{
	return GetCharacterMovement()->Velocity;
}

void AStarlightCharacter::SetVelocity(const FVector& Velocity)
{
	GetCharacterMovement()->Velocity = Velocity;
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

FRotator AStarlightCharacter::GetTeleportRotation()
{
	return GetControlRotation();
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
	for (APortal* Portal : OverlappingPortals)
	{
		Portal->OnActorMoved(this);
	}
}

