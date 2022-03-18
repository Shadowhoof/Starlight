// Shadowhoof Games, 2022


#include "Portal/Portal.h"

#include "Camera/CameraComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "GameFramework/Character.h"
#include "Portal/PortalConstants.h"
#include "Portal/PortalSurface.h"


UCharacterInPortalRange::UCharacterInPortalRange()
{
}

void UCharacterInPortalRange::Initialize(TObjectPtr<ACharacter> InCharacter, TObjectPtr<APortal> InPortal)
{
	ensure(InCharacter && InPortal);
	Character = InCharacter;
	Portal = InPortal;

	Character->OnCharacterMovementUpdated.AddDynamic(this, &UCharacterInPortalRange::OnCharacterMoved);
}

void UCharacterInPortalRange::Deinitialize()
{
	Character->OnCharacterMovementUpdated.RemoveDynamic(this, &UCharacterInPortalRange::OnCharacterMoved);
	Character = nullptr;
	Portal = nullptr;
}

void UCharacterInPortalRange::OnCharacterMoved(float DeltaSeconds, FVector OldLocation, FVector OldVelocity)
{
	Portal->OnCharacterMoved(Character);
}

APortal::APortal()
{
	PrimaryActorTick.bCanEverTick = true;

	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = PortalMesh;

	BorderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BorderMesh"));
	BorderMesh->SetupAttachment(RootComponent);

	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	SceneCaptureComponent->bCaptureEveryFrame = true;
	SceneCaptureComponent->bOverride_CustomNearClippingPlane = true;
	SceneCaptureComponent->SetupAttachment(RootComponent);

	CollisionBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBoxComponent"));
	CollisionBoxComponent->SetBoxExtent(PortalConstants::BorderCollisionExtent);
	CollisionBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	CollisionBoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	CollisionBoxComponent->SetCollisionResponseToChannel(ECC_PhysicsBody, ECR_Overlap);
	CollisionBoxComponent->SetGenerateOverlapEvents(true);
	CollisionBoxComponent->SetupAttachment(RootComponent);

	BackfacingComponent = CreateDefaultSubobject<USceneComponent>(TEXT("BackFacingComponent"));
	BackfacingComponent->SetRelativeRotation(FRotator(0.f, 180.f, 0.f));
	BackfacingComponent->SetupAttachment(RootComponent);
}

void APortal::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!OtherPortal || ActorsInPortalRange.Num() == 0)
	{
		return;
	}

	const FVector PortalNormal = GetActorRotation().Vector();
	TArray<TObjectPtr<AActor>> TeleportingActors;
	for (TObjectPtr<AActor> Actor : ActorsInPortalRange)
	{
		if (ShouldTeleportActor(Actor, PortalNormal))
		{
			TeleportingActors.Add(Actor);
		}
	}

	for (TObjectPtr<AActor> Actor : TeleportingActors)
	{
		TeleportActor(Actor);
	}
}

void APortal::Initialize(const TObjectPtr<APortalSurface> Surface, FVector InLocalCoords)
{
	if (PortalSurface)
	{
		UE_LOG(LogPortal, Error, TEXT("PortalSurface is already set for portal %s"), *GetName());
		return;
	}

	if (!Surface)
	{
		UE_LOG(LogPortal, Error, TEXT("PortalSurface is nullptr for portal %s"), *GetName());
		return;
	}

	PortalSurface = Surface;
	LocalCoords = InLocalCoords;
}

TObjectPtr<APortalSurface> APortal::GetPortalSurface() const
{
	return PortalSurface;
}

FVector APortal::GetLocalCoords() const
{
	return LocalCoords;
}

void APortal::SetRenderTargets(TObjectPtr<UTextureRenderTarget2D> ReadTarget,
                               TObjectPtr<UTextureRenderTarget2D> WriteTarget)
{
	if (!ReadTarget || !WriteTarget)
	{
		UE_LOG(LogPortal, Error, TEXT("APortal::SetRenderTargets | Either ReadTarget or WriteTarget is nullptr"));
		return;
	}

	RenderTargetRead = ReadTarget;
	DynamicInstance->SetTextureParameterValue("PortalTexture", RenderTargetRead);
	PortalMesh->SetMaterial(0, DynamicInstance);

	RenderTargetWrite = WriteTarget;
	SceneCaptureComponent->TextureTarget = WriteTarget;
}

void APortal::UpdateSceneCaptureTransform(const FTransform& RelativeTransform)
{
	SceneCaptureComponent->CustomNearClippingPlane = RelativeTransform.GetTranslation().Length();
	SceneCaptureComponent->SetRelativeTransform(RelativeTransform);
}

FTransform APortal::GetBackfacingRelativeTransform(TObjectPtr<ACharacter> PlayerCharacter) const
{
	const FTransform ViewTransform = {PlayerCharacter->GetControlRotation(), PlayerCharacter->GetPawnViewLocation()};
	return ViewTransform.GetRelativeTransform(BackfacingComponent->GetComponentTransform());
}

void APortal::SetConnectedPortal(TObjectPtr<APortal> Portal)
{
	OtherPortal = Portal;
}

void APortal::PrepareForActorTeleport(TObjectPtr<AActor> TeleportingActor)
{
	if (OtherPortal->GetPortalSurface() != PortalSurface)
	{
		OnActorBeginOverlap(TeleportingActor);
	}
}

void APortal::OnCharacterMoved(TObjectPtr<ACharacter> Character)
{
	if (ShouldTeleportActor(Character))
	{
		TeleportActor(Character);
	}
}

void APortal::BeginPlay()
{
	Super::BeginPlay();

	DynamicInstance = UMaterialInstanceDynamic::Create(PortalMesh->GetMaterial(0), this);

	CollisionBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &APortal::OnCollisionBoxStartOverlap);
	CollisionBoxComponent->OnComponentEndOverlap.AddDynamic(this, &APortal::OnCollisionBoxEndOverlap);
}

void APortal::OnCollisionBoxStartOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                         UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep,
                                         const FHitResult& SweepResult)
{
	OnActorBeginOverlap(OtherActor);
}

void APortal::OnCollisionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                       UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	OnActorEndOverlap(OtherActor);
}

void APortal::OnActorBeginOverlap(TObjectPtr<AActor> Actor)
{
	if (!ActorsInPortalRange.Contains(Actor))
	{
		UE_LOG(LogPortal, Verbose, TEXT("Portal %s is now overlapping with %s"), *GetName(), *Actor->GetName());
		PortalSurface->SetCollisionEnabledForActor(Actor, false);
		ActorsInPortalRange.Add(Actor);

		if (TObjectPtr<ACharacter> Character = Cast<ACharacter>(Actor))
		{
			TObjectPtr<UCharacterInPortalRange> CharacterInRange = NewObject<UCharacterInPortalRange>(this);
			CharacterInRange->Initialize(Character, this);
			CharactersInPortalRange.Add(Character, CharacterInRange);
		}
	}
}

void APortal::OnActorEndOverlap(TObjectPtr<AActor> Actor)
{
	UE_LOG(LogPortal, Verbose, TEXT("Portal %s is no longer overlapping with %s"), *GetName(), *Actor->GetName());
	PortalSurface->SetCollisionEnabledForActor(Actor, true);
	ActorsInPortalRange.Remove(Actor);

	if (TObjectPtr<ACharacter> Character = Cast<ACharacter>(Actor))
	{
		TObjectPtr<UCharacterInPortalRange> CharacterInRange = CharactersInPortalRange[Character];
		CharacterInRange->Deinitialize();
		CharactersInPortalRange.Remove(Character);
	}
}

void APortal::TeleportActor(TObjectPtr<AActor> Actor)
{
	const TObjectPtr<APawn> Pawn = Cast<APawn>(Actor);
	const FRotator CurrentRotation = Pawn ? Pawn->GetControlRotation() : GetActorRotation();
	const FRotator RelativeRotation = CurrentRotation - BackfacingComponent->GetComponentRotation();
	const FRotator NewRotation = OtherPortal->GetActorRotation() + RelativeRotation;
	// TODO: this probably will have to be reworked slightly for wall-to-floor portals
	const FVector RelativeLocation = BackfacingComponent->GetComponentTransform().InverseTransformPosition(
		Actor->GetActorLocation());
	const FVector NewLocation = OtherPortal->GetTransform().TransformPosition(RelativeLocation);
	OtherPortal->PrepareForActorTeleport(Actor);
	Actor->TeleportTo(NewLocation, NewRotation);
	UE_LOG(LogPortal, Verbose, TEXT("Portal %s has teleported actor %s"), *GetName(), *Actor->GetName());
}

bool APortal::ShouldTeleportActor(TObjectPtr<AActor> Actor, const FVector PortalNormal) const
{
	const FVector ToActorDirection = (Actor->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	return ToActorDirection.Dot(PortalNormal) < 0.f;
}

bool APortal::ShouldTeleportActor(TObjectPtr<AActor> Actor) const
{
	return ShouldTeleportActor(Actor, GetActorRotation().Vector());
}
