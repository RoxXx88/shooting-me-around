#include "ShootMeAround.h"
#include "ShootMeAroundCharacter.h"

#include "GameFramework/Actor.h"
#include "GameFramework/PlayerController.h"

#include "Components/BillboardComponent.h"

#include "Engine/StaticMeshActor.h"

#include "ComponentController.h"

AComponentController::AComponentController()
{
	PrimaryActorTick.bCanEverTick = true;

	Billboard = CreateDefaultSubobject<UBillboardComponent>(TEXT("Billboard"));

	RightTranslation = 0.f;
	FrontTranslation = 0.f;
}

void AComponentController::AddBullet(AActor* NewBullet)
{
	OriginalBulletInstances.Add(NewBullet);
}

void AComponentController::BeginPlay()
{
	Super::BeginPlay();

	FillOriginalObjectsArray();
}

void AComponentController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TeleportObjectsIfEscaping();

	UpdateTriggerLimit();
}

void AComponentController::FillOriginalObjectsArray()
{
	FConstPlayerControllerIterator PlayerControllers = GetWorld()->GetPlayerControllerIterator();
	
	for (int PlayerControllerIndex = 0; PlayerControllerIndex < GetWorld()->PlayerNum; ++PlayerControllerIndex, ++PlayerControllers)
	{
		TAutoWeakObjectPtr<APlayerController> ActualPlayer = *PlayerControllers;
		ACharacter* PlayerCharacter = ActualPlayer->GetCharacter();
		OriginalCharacterInstances.Add(PlayerCharacter);
	}

	for (AActor *Object : Objects)
	{
		OriginalObjectInstances.Add(Object);
	}
}

void AComponentController::TeleportObjectsIfEscaping()
{
	for (AActor* Object : OriginalObjectInstances)
	{
		TeleportIfEscaping(Object);
	}

	for (AActor* Object : OriginalCharacterInstances)
	{
		TeleportIfEscaping(Object);
	}

	for (AActor* Object : OriginalBulletInstances)
	{
		TeleportIfEscaping(Object);
	}
}

void AComponentController::TeleportIfEscaping(AActor* Object)
{
	if (Object == nullptr || !Object->IsValidLowLevel())
	{
		return;
	}

	FVector CurrLocation = Object->GetActorLocation();

	// Check top.

	if (CurrLocation.X > FrontTranslation / 2.0f)
	{
		FVector NewLocation = CurrLocation;
		NewLocation.X -= FrontTranslation;

   		Object->SetActorLocation(NewLocation);
	}
	else if (CurrLocation.X < -FrontTranslation / 2.0f) // Check bottom.
	{
		FVector NewLocation = CurrLocation;
		NewLocation.X += FrontTranslation;

		Object->SetActorLocation(NewLocation);
	}

	// Check right.

	if (CurrLocation.Y > RightTranslation / 2.0f)
	{
		FVector NewLocation = CurrLocation;
		NewLocation.Y -= RightTranslation;

		Object->SetActorLocation(NewLocation);
	}
	else if (CurrLocation.Y < -RightTranslation / 2.0f) // Check left.
	{
		FVector NewLocation = CurrLocation;
		NewLocation.Y += RightTranslation;

		Object->SetActorLocation(NewLocation);
	}
}

void AComponentController::UpdateTriggerLimit()
{
	RightTranslation = World->GetComponentsBoundingBox().GetSize().Y;
	FrontTranslation = World->GetComponentsBoundingBox().GetSize().X;

	float WidthRadius = FrontTranslation /2.0f;
	float HeightRadius = RightTranslation / 2.0f;

	FVector WorldLocation = World->GetActorLocation();

	float TriggerLimitZ = PositiveX->GetActorLocation().Z;

	PositiveX->SetActorLocation({ WorldLocation.X + WidthRadius, WorldLocation.Y, TriggerLimitZ });
	PositiveY->SetActorLocation({ WorldLocation.X, WorldLocation.Y + HeightRadius, TriggerLimitZ });

	NegativeX->SetActorLocation({ WorldLocation.X - WidthRadius, WorldLocation.Y, TriggerLimitZ });
	NegativeY->SetActorLocation({ WorldLocation.X, WorldLocation.Y - HeightRadius, TriggerLimitZ });
}