// Shadowhoof Games, 2022


#include "Portal/Portal.h"

#include "Camera/CameraComponent.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Portal/PortalConstants.h"


APortal::APortal()
{
	PortalMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	PortalMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	RootComponent = PortalMesh;

	BorderMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BorderMesh"));
	BorderMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	BorderMesh->SetupAttachment(RootComponent);

	SceneCaptureComponent = CreateDefaultSubobject<USceneCaptureComponent2D>(TEXT("SceneCaptureComponent"));
	SceneCaptureComponent->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
	SceneCaptureComponent->bCaptureEveryFrame = true;
	SceneCaptureComponent->SetupAttachment(RootComponent);
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

void APortal::BeginPlay()
{
	Super::BeginPlay();

	DynamicInstance = UMaterialInstanceDynamic::Create(PortalMesh->GetMaterial(0), this);
}
